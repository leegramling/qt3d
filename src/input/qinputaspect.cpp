/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qinputaspect.h"
#include "qinputaspect_p.h"
#include "cameracontroller_p.h"
#include "inputhandler_p.h"
#include "keyboardcontroller_p.h"
#include "keyboardinput_p.h"
#include <Qt3DCore/qaspectfactory.h>
#include <Qt3DCore/qnodevisitor.h>
#include <Qt3DCore/qscenepropertychange.h>
#include <Qt3DInput/qkeyboardcontroller.h>
#include <Qt3DInput/qkeyboardinput.h>

QT_BEGIN_NAMESPACE

namespace Qt3D {

QInputAspectPrivate::QInputAspectPrivate(QInputAspect *qq)
    : QAbstractAspectPrivate(qq)
    , m_inputHandler(new Input::InputHandler())
    , m_cameraController(new Input::CameraController())
{
    m_aspectType = QAbstractAspect::AspectOther;
}

QInputAspect::QInputAspect(QObject *parent)
    : QAbstractAspect(*new QInputAspectPrivate(this), parent)
{
    registerBackendType<QKeyboardController>(QBackendNodeFunctorPtr(new Input::KeyboardControllerFunctor(d_func()->m_inputHandler)));
    registerBackendType<QKeyboardInput>(QBackendNodeFunctorPtr(new Input::KeyboardInputFunctor(d_func()->m_inputHandler)));
}

QCamera *QInputAspect::camera() const
{
    Q_D(const QInputAspect);
    return d->m_cameraController->camera();
}

void QInputAspect::setCamera(QCamera *camera)
{
    Q_D(QInputAspect);
    d->m_cameraController->setCamera(camera);
}

QVector<QAspectJobPtr> QInputAspect::jobsToExecute(qint64 time)
{
    Q_UNUSED(time);
    Q_D(QInputAspect);
    QVector<QAspectJobPtr> jobs;

    jobs.append(d->m_inputHandler->keyboardJobs());
    // One job for Mouse events

    return jobs;
}

void QInputAspect::sceneNodeAdded(QSceneChangePtr &e)
{
    QScenePropertyChangePtr propertyChange = e.staticCast<QScenePropertyChange>();
    QNodePtr nodePtr = propertyChange->value().value<QNodePtr>();
    QNode *n = nodePtr.data();
    QNodeVisitor visitor;
    visitor.traverse(n, this, &QInputAspect::visitNode, &QInputAspect::visitNode);
}

void QInputAspect::sceneNodeRemoved(QSceneChangePtr &e)
{
    QScenePropertyChangePtr propertyChange = e.staticCast<QScenePropertyChange>();
    QNodePtr nodePtr = propertyChange->value().value<QNodePtr>();
    QNode *n = nodePtr.data();
    QAbstractAspect::clearBackendNode(n);
}

void QInputAspect::setRootEntity(QEntity *rootObject)
{
    QNodeVisitor visitor;
    visitor.traverse(rootObject, this, &QInputAspect::visitNode, &QInputAspect::visitNode);
}

void QInputAspect::onInitialize(const QVariantMap &data)
{
    QObject *object = Q_NULLPTR;
    const QVariant &v = data.value(QStringLiteral("eventSource"));
    if (v.isValid())
        object = v.value<QObject *>();
    Q_D(QInputAspect);
    if (object)
        object->installEventFilter(d->m_cameraController);
    d->m_inputHandler->setEventSource(object);
}

void QInputAspect::onCleanup()
{
    Q_D(QInputAspect);
    d->m_inputHandler->eventSource()->removeEventFilter(d->m_cameraController);
    d->m_inputHandler->setEventSource(Q_NULLPTR);
}

void QInputAspect::visitNode(QNode *node)
{
    QAbstractAspect::createBackendNode(node);
}

} // Qt3D

QT_END_NAMESPACE

QT3D_REGISTER_NAMESPACED_ASPECT("input", QT_PREPEND_NAMESPACE(Qt3D), QInputAspect)
