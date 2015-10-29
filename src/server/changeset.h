/****************************************************************************
 * Copyright 2015  Sebastian Kügler <sebas@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#ifndef WAYLAND_SERVER_CHANGESET_H
#define WAYLAND_SERVER_CHANGESET_H

#include <QObject>

#include "outputdevice_interface.h"
#include <KWayland/Server/kwaylandserver_export.h>

namespace KWayland
{
namespace Server
{

/**
 * @brief Holds a set of changes to an OutputInterface or OutputDeviceInterface.
 *
 * This class implements a set of changes that the compositor can apply to an
 * OutputInterface after OutputConfiguration::apply has been called on the client
 * side. The changes are per-configuration.
 *
 * @see OutputConfiguration
 *
 **/
class KWAYLANDSERVER_EXPORT ChangeSet : public QObject
{
    Q_OBJECT
public:
    explicit ChangeSet(QObject *parent = nullptr);
    virtual ~ChangeSet();

    bool enabledChanged() const;
    bool modeChanged() const;
    bool transformChanged() const;
    bool positionChanged() const;
    bool scaleChanged() const;

    OutputDeviceInterface::Enablement enabled() const;
    int mode() const;
    OutputDeviceInterface::Transform transform() const;
    QPoint position() const;
    int scale() const;

protected:
    friend class OutputConfiguration;
    void setEnabled(OutputDeviceInterface::Enablement enablement);
    void setMode(int modeId);
    void setTransform(OutputDeviceInterface::Transform);
    void setPosition(QPoint pos);
    void setScale(int scale);

    class Private;
    QScopedPointer<Private> d;
    Private *d_func() const;
};

}
}

#endif
