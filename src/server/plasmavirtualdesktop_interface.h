/****************************************************************************
Copyright 2018  Marco Martin <notmart@gmail.com>

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
#ifndef KWAYLAND_SERVER_PLASMAVIRTUALDESKTOP_H
#define KWAYLAND_SERVER_PLASMAVIRTUALDESKTOP_H

#include "global.h"
#include "resource.h"

#include <KWayland/Server/kwaylandserver_export.h>

namespace KWayland
{
namespace Server
{

class Display;
class PlasmaVirtualDesktopInterface;

/**
 * @short Wrapper for the org_kde_plasma_virtual_desktop_management interface.
 *
 * This class provides a convenient wrapper for the org_kde_plasma_virtual_desktop_management interface.
 * @since 5.46
 */
class KWAYLANDSERVER_EXPORT PlasmaVirtualDesktopManagementInterface : public Global
{
    Q_OBJECT
public:
    virtual ~PlasmaVirtualDesktopManagementInterface();

    /**
     * Sets a new layout for this desktop grid.
     */
    void setLayout(quint32 rows, quint32 columns);

    /**
     * @returns A desktop identified uniquely by this id.
     * If not found, nullptr will be returned.
     * @see createDesktop
     */
    PlasmaVirtualDesktopInterface *desktop(const QString &id);

    /**
     * @returns A desktop identified uniquely by this id, if not found
     * a new desktop will be created for this id.
     */
    PlasmaVirtualDesktopInterface *createDesktop(const QString &id);

    /**
     * Removed and destroys the desktop identified by id, if present
     */
    void removeDesktop(const QString &id);

    /**
     * @returns All tghe desktops present.
     */
    QList <PlasmaVirtualDesktopInterface *> desktops() const;

    /**
     * Inform the clients that all the properties have been sent, and
     * their client-side representation is complete.
     */
    void sendDone();

    /**
     * FIXME: RFC this assumes there is only one desktop active at once, may need to be removed if we ever want to support per-screen desktops, or, a second string param could be added as a context for active: that could be the name of an output, or whatever else needed
     * Sets the desktop identified by id to be the active one.
     * active desktops are mutually exclusive
     */
    void setActiveDesktop(const QString &id);

Q_SIGNALS:
    /**
     * A desktop has been activated
     */
    void desktopActivated(const QString &id);

    /**
     * The client asked to remove a desktop, It's responsibility of the server
     * deciding whether to remove it or not.
     */
    void desktopRemoveRequested(const QString &id);

private:
    explicit PlasmaVirtualDesktopManagementInterface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
    Private *d_func() const;
};

class KWAYLANDSERVER_EXPORT PlasmaVirtualDesktopInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~PlasmaVirtualDesktopInterface();

    /**
     * @returns the unique id for this desktop.
     * ids are set at creation time by PlasmaVirtualDesktopManagementInterface::createDesktop
     * and can't be changed at runtime.
     */
    QString id() const;

    /**
     * Sets a new name for this desktop
     */
    void setName(const QString &name);

    /**
     * @returns the name for this desktop
     */
    QString name() const;

    /**
     * @returns true if this desktop is active. Only one at a time will be.
     */
    bool active() const;

    /**
     * Inform the clients that all the properties have been sent, and
     * their client-side representation is complete.
     */
    void sendDone();

Q_SIGNALS:
    /**
     * Emitted when the client asked to activate this desktop:
     * it's the decision of the server whether to perform the activation or not.
     */
    void activateRequested();

private:
    explicit PlasmaVirtualDesktopInterface(PlasmaVirtualDesktopManagementInterface *parent);
    friend class PlasmaVirtualDesktopManagementInterface;

    class Private;
    const QScopedPointer<Private> d;
};

}
}

#endif
