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
#include "plasmavirtualdesktop.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"

#include <QMap>
#include <QDebug>

#include <wayland-plasma-virtual-desktop-client-protocol.h>

namespace KWayland
{
namespace Client
{

class PlasmaVirtualDesktopManagement::Private
{
public:
    Private(PlasmaVirtualDesktopManagement *q);

    void setup(org_kde_plasma_virtual_desktop_management *arg);

    WaylandPointer<org_kde_plasma_virtual_desktop_management, org_kde_plasma_virtual_desktop_management_destroy> plasmavirtualdesktopmanagement;
    EventQueue *queue = nullptr;

    //is a map to have desktops() return a list always with the same order
    QMap<QString, PlasmaVirtualDesktop *> desktops;

private:
    static void addedCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, const char *id);
    static void removedCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, const char *id);
    static void doneCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management);

    PlasmaVirtualDesktopManagement *q;

    static const org_kde_plasma_virtual_desktop_management_listener s_listener;
};

const org_kde_plasma_virtual_desktop_management_listener PlasmaVirtualDesktopManagement::Private::s_listener = {
    addedCallback,
    removedCallback,
    doneCallback
};

void PlasmaVirtualDesktopManagement::Private::addedCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, const char *id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktopManagement::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktopmanagement == org_kde_plasma_virtual_desktop_management);
    const QString stringId = QString::fromUtf8(id);
    PlasmaVirtualDesktop *vd = p->q->getVirtualDesktop(stringId);
    Q_ASSERT(vd);

    emit p->q->desktopAdded(stringId);
}

void PlasmaVirtualDesktopManagement::Private::removedCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, const char *id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktopManagement::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktopmanagement == org_kde_plasma_virtual_desktop_management);
    const QString stringId = QString::fromUtf8(id);
    PlasmaVirtualDesktop *vd = p->q->getVirtualDesktop(stringId);
    p->desktops.remove(stringId);
    Q_ASSERT(vd);
    vd->release();
    vd->destroy();
    vd->deleteLater();
    emit p->q->desktopRemoved(stringId);
}

void PlasmaVirtualDesktopManagement::Private::doneCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktopManagement::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktopmanagement == org_kde_plasma_virtual_desktop_management);
    emit p->q->done();
}

PlasmaVirtualDesktopManagement::PlasmaVirtualDesktopManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PlasmaVirtualDesktopManagement::Private::Private(PlasmaVirtualDesktopManagement *q)
    : q(q)
{}

void PlasmaVirtualDesktopManagement::Private::setup(org_kde_plasma_virtual_desktop_management *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!plasmavirtualdesktopmanagement);
    plasmavirtualdesktopmanagement.setup(arg);
    org_kde_plasma_virtual_desktop_management_add_listener(plasmavirtualdesktopmanagement, &s_listener, this);
}

PlasmaVirtualDesktopManagement::~PlasmaVirtualDesktopManagement()
{
    release();
}

void PlasmaVirtualDesktopManagement::setup(org_kde_plasma_virtual_desktop_management *plasmavirtualdesktopmanagement)
{
    d->setup(plasmavirtualdesktopmanagement);
}

void PlasmaVirtualDesktopManagement::release()
{
    d->plasmavirtualdesktopmanagement.release();
}

void PlasmaVirtualDesktopManagement::destroy()
{
    d->plasmavirtualdesktopmanagement.destroy();
}

PlasmaVirtualDesktopManagement::operator org_kde_plasma_virtual_desktop_management*() {
    return d->plasmavirtualdesktopmanagement;
}

PlasmaVirtualDesktopManagement::operator org_kde_plasma_virtual_desktop_management*() const {
    return d->plasmavirtualdesktopmanagement;
}

bool PlasmaVirtualDesktopManagement::isValid() const
{
    return d->plasmavirtualdesktopmanagement.isValid();
}

void PlasmaVirtualDesktopManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PlasmaVirtualDesktopManagement::eventQueue()
{
    return d->queue;
}

PlasmaVirtualDesktop *PlasmaVirtualDesktopManagement::getVirtualDesktop(const QString &id)
{
    Q_ASSERT(isValid());
    auto i = d->desktops.constFind(id);
    if (i != d->desktops.constEnd()) {
        return *i;
    }

    auto w = org_kde_plasma_virtual_desktop_management_get_virtual_desktop(d->plasmavirtualdesktopmanagement, id.toUtf8());

    if (!w) {
        return nullptr;
    }

    if (d->queue) {
        d->queue->addProxy(w);
    }

    auto desktop = new PlasmaVirtualDesktop(this);
    desktop->setup(w);
    d->desktops[id] = desktop;
    connect(desktop, &QObject::destroyed, this,
        [this, id] {
            d->desktops.remove(id);
        }
    );
    return desktop;
}

QList <PlasmaVirtualDesktop *> PlasmaVirtualDesktopManagement::desktops() const
{
    return d->desktops.values();
}

class PlasmaVirtualDesktop::Private
{
public:
    Private(PlasmaVirtualDesktop *q);

    void setup(org_kde_plasma_virtual_desktop *arg);

    WaylandPointer<org_kde_plasma_virtual_desktop, org_kde_plasma_virtual_desktop_destroy> plasmavirtualdesktop;

    QString id;
    QString name;
    QString topNeighbour;
    QString leftNeighbour;
    QString rightNeighbour;
    QString bottomNeighbour;
    bool active = false;

private:
    PlasmaVirtualDesktop *q;

private:
    static void idCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char * id);
    static void nameCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char * name);
    static void topNeighbourCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char *id);
    static void leftNeighbourCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char *id);
    static void rightNeighbourCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char *id);
    static void bottomNeighbourCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char *id);

    static void activatedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop);
    static void deactivatedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop);
    static void doneCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop);
    static void removedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop);

    static const org_kde_plasma_virtual_desktop_listener s_listener;
};

const org_kde_plasma_virtual_desktop_listener PlasmaVirtualDesktop::Private::s_listener = {
    idCallback,
    nameCallback,
    topNeighbourCallback,
    leftNeighbourCallback,
    rightNeighbourCallback,
    bottomNeighbourCallback,
    activatedCallback,
    deactivatedCallback,
    doneCallback,
    removedCallback
};

void PlasmaVirtualDesktop::Private::idCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char * id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    p->id = QString::fromUtf8(id);
}

void PlasmaVirtualDesktop::Private::nameCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char * name)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    p->name = QString::fromUtf8(name);
}

void PlasmaVirtualDesktop::Private::topNeighbourCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char *id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    const QString stringId = QString::fromUtf8(id);
    if (stringId != p->topNeighbour) {
        p->topNeighbour = stringId;
        emit p->q->topNeighbourChanged(stringId);
    }
}

void PlasmaVirtualDesktop::Private::leftNeighbourCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char *id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    const QString stringId = QString::fromUtf8(id);
    if (stringId != p->leftNeighbour) {
        p->leftNeighbour = stringId;
        emit p->q->leftNeighbourChanged(stringId);
    }
}

void PlasmaVirtualDesktop::Private::rightNeighbourCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char *id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    const QString stringId = QString::fromUtf8(id);
    if (stringId != p->rightNeighbour) {
        p->rightNeighbour = stringId;
        emit p->q->rightNeighbourChanged(stringId);
    }
}

void PlasmaVirtualDesktop::Private::bottomNeighbourCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char *id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    const QString stringId = QString::fromUtf8(id);
    if (stringId != p->bottomNeighbour) {
        p->bottomNeighbour = stringId;
        emit p->q->bottomNeighbourChanged(stringId);
    }
}

void PlasmaVirtualDesktop::Private::activatedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    p->active = true;
    emit p->q->activated();
}

void PlasmaVirtualDesktop::Private::deactivatedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    p->active = false;
    emit p->q->deactivated();
}

void PlasmaVirtualDesktop::Private::doneCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    emit p->q->done();
}

void PlasmaVirtualDesktop::Private::removedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    emit p->q->removed();
}

PlasmaVirtualDesktop::Private::Private(PlasmaVirtualDesktop *q)
    : q(q)
{
}

PlasmaVirtualDesktop::PlasmaVirtualDesktop(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void PlasmaVirtualDesktop::Private::setup(org_kde_plasma_virtual_desktop *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!plasmavirtualdesktop);
    plasmavirtualdesktop.setup(arg);
    org_kde_plasma_virtual_desktop_add_listener(plasmavirtualdesktop, &s_listener, this);
}

PlasmaVirtualDesktop::~PlasmaVirtualDesktop()
{
    release();
}

void PlasmaVirtualDesktop::setup(org_kde_plasma_virtual_desktop *plasmavirtualdesktop)
{
    d->setup(plasmavirtualdesktop);
}

void PlasmaVirtualDesktop::release()
{
    d->plasmavirtualdesktop.release();
}

void PlasmaVirtualDesktop::destroy()
{
    d->plasmavirtualdesktop.destroy();
}

PlasmaVirtualDesktop::operator org_kde_plasma_virtual_desktop*() {
    return d->plasmavirtualdesktop;
}

PlasmaVirtualDesktop::operator org_kde_plasma_virtual_desktop*() const {
    return d->plasmavirtualdesktop;
}

bool PlasmaVirtualDesktop::isValid() const
{
    return d->plasmavirtualdesktop.isValid();
}

void PlasmaVirtualDesktop::requestActivate()
{
    Q_ASSERT(isValid());
    org_kde_plasma_virtual_desktop_request_activate(d->plasmavirtualdesktop);
}

QString PlasmaVirtualDesktop::id() const
{
    return d->id;
}

QString PlasmaVirtualDesktop::name() const
{
    return d->name;
}

QString PlasmaVirtualDesktop::topNeighbour() const
{
    return d->topNeighbour;
}

QString PlasmaVirtualDesktop::leftNeighbour() const
{
    return d->leftNeighbour;
}

QString PlasmaVirtualDesktop::rightNeighbour() const
{
    return d->rightNeighbour;
}

QString PlasmaVirtualDesktop::bottomNeighbour() const
{
    return d->bottomNeighbour;
}

bool PlasmaVirtualDesktop::active() const
{
    return d->active;
}

}
}

