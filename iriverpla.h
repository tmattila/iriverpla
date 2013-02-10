#ifndef IRIVERPLA_H
#define IRIVERPLA_H

#include <QMainWindow>
class plaPlayList;

namespace Ui {
    class IRiverPla;
}

/**
 * IRiverPLA class offers GUI services.
 *
 * Currently available services:
 * \li selecting tracks
 * \li ordering selected tracks
 * \li log output for debug purposes (CTRL+L)
 * \li 'settings', music files destination, root under which all music files exists, playlist destination,...
 * \li creating PLA file and copying it to destination
 * \li drop support for adding files (checks playlist file type support)
 *
 * Two classes to work with:
 * \li IRiverPLA, very thin GUI class to offer interaction with user (should be easily replacable for ex. with QML?)
 * \li plaPlaylist, class that takes care of all hard work and hides PLA implementation details.
 *
 * Author 2013 Tapio Mattila
 * Code can be freely used and modified. Make this program better so we can all enjoy of it!
 * No warranties given what so ever, so use it at your own risk.
 */
class IRiverPla : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit IRiverPla(QWidget *parent = 0);
    ~IRiverPla();

private slots:
    void on_action_Add_to_playlist_triggered();
    void on_actionIriver_Plus_triggered();
    void on_actionShow_Log_triggered();
    void on_actionPlaylist_Destination_triggered();
    void on_actionMusic_destination_triggered();
    void on_actionRemove_triggered();
    void on_actionGenerate_triggered();
    void on_btnAdd_clicked();
    void on_btnDestination_clicked();
    void on_btnGenerate_clicked();
    void on_btnUp_clicked();
    void on_btnDown_clicked();
    void on_btnPlaylistdestination_clicked();
    void on_btnRemove_clicked();
    void on_cbKeepFolder_toggled(bool checked);
    void repositionItems(Qt::SortOrder);
    void playListError(QString, QString, QString);
    void addFilesToPlaylist(QStringList files);
    void playListReady();

protected:
    void dropEvent(QDropEvent *);
    void dragEnterEvent(QDragEnterEvent*);

private:
    QString previousAddMusicPath;
    Ui::IRiverPla *ui;
    plaPlayList *playList;
};

#endif // IRIVERPLA_H
