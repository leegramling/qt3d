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

#include "qlayer.h"
#include "qlayer_p.h"

QT_BEGIN_NAMESPACE

namespace Qt3D {

QLayerPrivate::QLayerPrivate(QLayer *qq)
    : QComponentPrivate(qq)
{
}

void QLayer::copy(const QNode *ref)
{
    QComponent::copy(ref);
    const QLayer *layer = static_cast<const QLayer*>(ref);
    d_func()->m_names = layer->d_func()->m_names;
}

QLayer::QLayer(QNode *parent)
    : QComponent(*new QLayerPrivate(this), parent)
{
}

QLayer::QLayer(QLayerPrivate &dd, QNode *parent)
    : QComponent(dd, parent)
{
}

QStringList QLayer::names() const
{
    Q_D(const QLayer);
    return d->m_names;
}

void QLayer::setNames(const QStringList &names)
{
    Q_D(QLayer);
    if (d->m_names != names) {
        d->m_names = names;
        emit namesChanged();
    }
}

} // Qt3D

QT_END_NAMESPACE
