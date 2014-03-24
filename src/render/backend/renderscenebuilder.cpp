/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderscenebuilder.h"

#include "meshmanager.h"
#include "renderer.h"
#include "rendernode.h"

#include <camera.h>
#include <material.h>
#include <mesh.h>
#include <transform.h>
#include <entity.h>

#include <framegraph.h>
#include <viewport.h>
#include <techniquefilter.h>
#include <renderpassfilter.h>
#include <cameraselector.h>

#include <techniquefilternode.h>
#include <cameraselectornode.h>
#include <renderpassfilternode.h>
#include <viewportnode.h>


namespace Qt3D {
namespace Render {

RenderSceneBuilder::RenderSceneBuilder(Renderer *renderer)
    : Qt3D::NodeVisitor()
    , m_renderer(renderer)
    , m_rootNode(0)
{
}

/*!
 * Returns a FrameGraphNode and all its children from \a node which points to the activeFrameGraph.
 * Returns Q_NULLPTR if \a is also Q_NULLPTR or if there is no FrameGraphComponent
 * in node's children tree.
 */
Render::FrameGraphNode *RenderSceneBuilder::buildFrameGraph(Node *node)
{
    if (node == Q_NULLPTR)
        return Q_NULLPTR;

    qDebug() << Q_FUNC_INFO << node->objectName();

    Render::FrameGraphNode *fgNode = Q_NULLPTR;
    Qt3D::FrameGraphItem *fgItem = Q_NULLPTR;

    if ((fgItem = qobject_cast<Qt3D::FrameGraphItem *>(node)) != Q_NULLPTR) {
        // Instantiate proper backend node corresponding to the frontend node
        fgNode = backendFrameGraphNode(fgItem);
    }

    // we need to travel the node's children tree to find either the FG root Node or
    // its children
    QList<FrameGraphNode *> fgChildNodes;
    foreach (Node *child, node->children()) {
        FrameGraphNode* fgChildNode = buildFrameGraph(child);
        if (fgChildNode != Q_NULLPTR)
            fgChildNodes << fgChildNode;
    }
    if (!fgChildNodes.isEmpty()) {
        if (fgNode == Q_NULLPTR && fgChildNodes.size() == 1) {
            fgNode = fgChildNodes.first();
        }
        else {
            if (fgNode == Q_NULLPTR)
                fgNode = new FrameGraphNode();
            foreach (FrameGraphNode *fgChildNodes, fgChildNodes)
                fgNode->appendChild(fgChildNodes);
        }
    }
    return fgNode;
}

/*!
 * Returns a proper FrameGraphNode subclass instance from \a block.
 * If no subclass corresponds, Q_NULLPTR is returned.
 */
Render::FrameGraphNode *RenderSceneBuilder::backendFrameGraphNode(Qt3D::FrameGraphItem *block)
{
    if (qobject_cast<Qt3D::TechniqueFilter*>(block) != Q_NULLPTR) {
        qDebug() << Q_FUNC_INFO << "TechniqueFilter";
        Render::TechniqueFilter *techniqueFilterNode = new Render::TechniqueFilter();
        return techniqueFilterNode;
    }
    else if (qobject_cast<Qt3D::Viewport*>(block) != Q_NULLPTR) {
        qDebug() << Q_FUNC_INFO << "Viewport";
        Render::ViewportNode *viewportNode = new Render::ViewportNode();
        return viewportNode;
    }
    else if (qobject_cast<Qt3D::RenderPassFilter*>(block) != Q_NULLPTR) {
        qDebug() << Q_FUNC_INFO << "RenderPassFilter";
        Render::RenderPassFilter *renderPassFilterNode = new Render::RenderPassFilter();
        return renderPassFilterNode;
    }
    else if (qobject_cast<Qt3D::CameraSelector*>(block) != Q_NULLPTR)
    {
        qDebug() << Q_FUNC_INFO << "CameraSelector";
        Render::CameraSelector *cameraSelectorNode = new Render::CameraSelector();
        return cameraSelectorNode;
    }
    return Q_NULLPTR;
}

void RenderSceneBuilder::visitNode(Qt3D::Node *node)
{
    if (!m_rootNode) {
        m_rootNode = new RenderNode(m_renderer->rendererAspect());
        m_rootNode->m_frontEndPeer = node;
        m_nodeStack.push(m_rootNode);
    }
    qDebug() << Q_FUNC_INFO << "Node " << node->objectName();
    Qt3D::NodeVisitor::visitNode(node);
}

void RenderSceneBuilder::visitEntity(Qt3D::Entity *entity)
{
    // Create a RenderNode corresponding to the Entity. Most data will
    // be calculated later by jobs
    qDebug() << Q_FUNC_INFO << "Entity " << entity->objectName();
    RenderNode *renderNode = new RenderNode(m_renderer->rendererAspect(), m_nodeStack.top());
//    entity->dumpObjectTree();
    renderNode->m_frontEndPeer = entity;
    // REPLACE WITH ENTITY MATRIX FROM TRANSFORMS
//    *(renderNode->m_localTransform) = entity->matrix();
    m_nodeStack.push(renderNode);

    // Look for a transform component
    QList<Transform *> transforms = entity->componentsOfType<Transform>();
    if (!transforms.isEmpty())
        renderNode->setPeer(transforms.first());


    QList<FrameGraph *> framegraphRefs = entity->componentsOfType<FrameGraph>();
    if (!framegraphRefs.isEmpty()) {
        FrameGraph *fg = framegraphRefs.first();
        // Entity has a reference to a framegraph configuration
        // Build a tree of FrameGraphNodes by reading the tree of FrameGraphBuildingBlocks
        Render::FrameGraphNode* frameGraphRootNode = buildFrameGraph(fg->activeFrameGraph());
        qDebug() << Q_FUNC_INFO << "SceneGraphRoot" <<  frameGraphRootNode;
        m_renderer->setFrameGraphRoot(frameGraphRootNode);
    }

    //    QList<Material *> materials = entity->componentsOfType<Material>();
    //    Material *material = 0;
    //    if (!materials.isEmpty())
    //        material = materials.first();

    // We'll update matrices in a job later. In fact should the matrix be decoupled from the mesh?
    foreach (Mesh *mesh, entity->componentsOfType<Mesh>()) {
        m_renderer->meshManager()->addMesh(mesh);
        //m_renderer->buildMeshes(mesh, material, QMatrix4x4());
    }

    //foreach (Camera *camera, entity->componentsOfType<Camera>())
    //    m_renderer->foundCamera(camera, entity->sceneMatrix());

    NodeVisitor::visitEntity(entity);

    // Coming back up the tree
    m_nodeStack.pop();
}

} // namespace Render
} // namespace Qt3D
