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

#ifndef QT3D_CAMERASELECTOR_H
#define QT3D_CAMERASELECTOR_H

#include <Qt3DRenderer/qt3drenderer_global.h>
#include <Qt3DRenderer/framegraphitem.h>

QT_BEGIN_NAMESPACE

namespace Qt3D {

class Camera;
class CameraSelector;

class CameraSelectorPrivate
{
public:
    CameraSelectorPrivate(CameraSelector *qq)
        : q_ptr(qq)
        , m_camera(Q_NULLPTR)
    {}

    Q_DECLARE_PUBLIC(CameraSelector)
    CameraSelector *q_ptr;
    Node *m_camera;
};

class QT3DRENDERERSHARED_EXPORT CameraSelector : public FrameGraphItem
{

public:
    CameraSelector()
        : d_ptr(new CameraSelectorPrivate(this))
    {}

    virtual ~CameraSelector() {}

    void setCamera(Qt3D::Node *camera)
    {
        Q_D(CameraSelector);
        if (d->m_camera != camera) {
          d->m_camera = camera;
          emit cameraChanged();
        }
    }

    Node *camera() const
    {
        Q_D(const CameraSelector);
        return d->m_camera;
    }

Q_SIGNALS:
    virtual void cameraChanged() = 0;

private:
    Q_DECLARE_PRIVATE(CameraSelector)
    CameraSelectorPrivate *d_ptr;
};

} // namespace Qt3D

Q_DECLARE_INTERFACE(Qt3D::CameraSelector, "org.qt-project.Qt3D.Render.CameraSelector/2.0")

QT_END_NAMESPACE


#endif // QT3D_CAMERASELECTOR_H