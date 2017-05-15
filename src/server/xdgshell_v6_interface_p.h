/****************************************************************************
Copyright 2017 David Edmundson <davidedmundson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/
#ifndef KWAYLAND_SERVER_XDGSHELL_V6_INTERFACE_P_H
#define KWAYLAND_SERVER_XDGSHELL_V6_INTERFACE_P_H

#include "global.h"
#include "resource.h"
#include "xdgshell_interface.h"

#include <KWayland/Server/kwaylandserver_export.h>

#include <QSize>

namespace KWayland
{
namespace Server
{

class Display;
class OutputInterface;
class SeatInterface;
class SurfaceInterface;
class XdgTopLevelV6Interface;
class XdgPopupV6Interface;
class XdgSurfaceV6Interface;
template <typename T>
class GenericShellSurface;

class XdgShellV6Interface : public XdgShellInterface
{
    Q_OBJECT
public:
    virtual ~XdgShellV6Interface();

    /**
     * @returns The XdgSurfaceV6Interface for the @p native resource.
     **/
    XdgTopLevelV6Interface *getSurface(wl_resource *native);

    Display *display() const;

    void ping();

private:
    explicit XdgShellV6Interface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
    Private *d_func() const;
};

class XdgSurfaceV6Interface : public KWayland::Server::Resource
{
    Q_OBJECT
public:
    virtual ~XdgSurfaceV6Interface();
    SurfaceInterface* surface() const;
    XdgTopLevelV6Interface* topLevel() const;
//     XdgPopupV6Interface *popup() const;

private:
    explicit XdgSurfaceV6Interface(XdgShellV6Interface *parent, SurfaceInterface *surface, wl_resource *parentResource);
    friend class XdgShellV6Interface;

    class Private;
    Private *d_func() const;
};

class XdgTopLevelV6Interface : public
XdgShellSurfaceInterface
{
    Q_OBJECT
public:
    virtual ~XdgTopLevelV6Interface();

private:
    //not really a direct parent.//mayeb also pass XdgShurfaceV6Interface as arg
    explicit XdgTopLevelV6Interface(XdgShellV6Interface *parent, SurfaceInterface *surface, wl_resource *parentResource);
    friend class XdgShellV6Interface;
    friend class XdgSurfaceV6Interface;

    class Private;
    Private *d_func() const;
};

class XdgPopupV6Interface : public XdgShellPopupInterface
{
    Q_OBJECT
public:
    virtual ~XdgPopupV6Interface();

private:
    explicit XdgPopupV6Interface(XdgShellV6Interface *parent, SurfaceInterface *surface, wl_resource *parentResource);
    friend class XdgShellV6Interface;
    friend class XdgSurfaceV6Interface;
    friend class GenericShellSurface<XdgPopupV6Interface>;

    class Private;
    Private *d_func() const;
};

class XdgPositionerV6Interface: public KWayland::Server::Resource
{
public:
private:
    explicit XdgPositionerV6Interface(XdgShellV6Interface *parent, wl_resource *parentResource);
    friend class XdgShellV6Interface;

    class Private;
    Private *d_func() const;
};

}
}

#endif
