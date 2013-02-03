/**
 * iRiverPLA, program to create iRiver playlist files (PLA format) and transfer them to
 * some disk (which can be also iRiver player).
 *
 * I needed this while iRiver did not supply such a program that would work in linux.
 * I have T20 which is a good player for me, but unfortunately track ordering did not
 * work at all in this device, hence this program.
 *
 * Features that would be nice to have:
 * \li done: create PLA file in correct format
 * \li done: support those music file formats that iRiver (T20) supports (not sure if this really matters, data is also good...)
 * \li done except folder selection missing: file selection in multiple ways (one/many selection (extended), drag and drop, folder selection)
 * \li done except ordering by dragging missing: define order of selected files (dragging, ordering button)
 * \li done: item(s) removal from selected list via button
 *
 * still needs implementation
 * \li selected file(s) existence and capacity check from iRiver device
 * \li selected file(s) copying to device if they are not there already
 * \li (?) settings to support backround threading (start synchronizing immediately when you know what and where)
 * \li some dynamic reporting that would show destination free capacity if all selections are synchronized
 *
 *
 * PLA format studied from Petteri Hintsanen web page: http://phintsan.kapsi.fi/iriver-t50.html
 *
 * Author 2013 Tapio Mattila
 * Code can be freely used and modified. Make this program better so we can all enjoy of it!
 * No warranties given what so ever, so use it at your own risk.
 */
#include <QApplication>
#include "iriverpla.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    IRiverPla w;
    w.show();
    return a.exec();
}
