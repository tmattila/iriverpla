#include "iriverpla.h"
#include "ui_iriverpla.h"
#include "plafile.h"

#include <QtGui>
#include <QDebug>

IRiverPla::IRiverPla(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::IRiverPla)
{
    ui->setupUi(this);
    // announce that this application (in general) accept drops even though they are only added to listwidget
    setAcceptDrops(true);
    playList = new plaPlayList(this);
    playList->preserveSongFolder = ui->cbKeepFolder->isChecked();
    ui->edtPlaylistName->setText(playList->playlistName);
    ui->edtLog->hide();
    connect(playList, SIGNAL(OnError(QString,QString,QString)), this, SLOT(playListError(QString,QString,QString)));
    connect(playList, SIGNAL(OnReady()), this, SLOT(playListReady()));
}

IRiverPla::~IRiverPla()
{
    delete playList;
    delete ui;
}


// Actions...

/**
 * \brief Define the destination into which playlist is copied.
 */
void IRiverPla::on_actionPlaylist_Destination_triggered()
{
    qDebug() << "IRiverPla::on_action_Destination_triggered()";
    QString destination = QFileDialog::getExistingDirectory(this, tr("Select location where playlist is saved"), playList->playlistDestination);
    if (!destination.isEmpty()) {
        playList->playlistDestination = destination;
    }
    qDebug() << "Playlist destination directory " << playList->playlistDestination;
}
/**
 * \brief Define the destination for Music files.
 */
void IRiverPla::on_actionMusic_destination_triggered()
{
    qDebug() << "IRiverPla::on_actionMusic_destination_triggered()";
    QString destination = QInputDialog::getText(this, tr("Give path to iRiver Music files (folders) location in relation to iRiver root"), tr("Path:"), QLineEdit::Normal, tr("\\Music"));
    if (!destination.isEmpty()) {
        destination = destination.replace("/", "\\");
        playList->musicFileDestination = destination;
    }
    qDebug() << "Destination directory for music " << playList->musicFileDestination;
}
/**
 * \brief Show program 'about box'.
 */
void IRiverPla::on_actionIriver_Plus_triggered()
{
    qDebug() << "IRiverPla::on_actionIriver_Plus_triggered()";
    QMessageBox::about(this, "About iriverPLA", "Program to create IRiver playlist files - PLA format");
}
/**
 * @brief IRiverPla::on_action_Add_to_playlist_triggered Adds a file or all supported files from selected folder to playlist
 * Method also checks that the same file is not already in the list (prevents dublicates)
 */
void IRiverPla::on_action_Add_to_playlist_triggered()
{
    qDebug() << "IRiverPla::on_action_Add_to_playlist_triggered()";
    try {
        QFileDialog::Options options;
        QString selectedFilter = "";
        if (previousAddMusicPath.isEmpty())
            previousAddMusicPath = QApplication::applicationDirPath();
        QStringList files = QFileDialog::getOpenFileNames(
                                    this, tr("Select files to playlist"),
                                    previousAddMusicPath,
                                    playList->getFileFilter(),
                                    &selectedFilter,
                                    options);
        if (files.isEmpty())
            return;
        previousAddMusicPath = QDir::cleanPath(files.at(0));
        addFilesToPlaylist(files);
    }
    catch (...) {
        ui->edtLog->append(("Error - IRiverPla::on_action_Add_to_playlist_triggered"));
    }
}
void IRiverPla::on_actionRemove_triggered()
{
    qDebug() << "IRiverPla::on_actionRemove_triggered()";
    QList<QListWidgetItem*> selectedItems = ui->lstFiles->selectedItems();
    for (int i=0; i<selectedItems.count(); i++) {
        delete ui->lstFiles->item(ui->lstFiles->row(selectedItems.at(i)));
    }
    QStringList remainingFiles;
    for (int i = 0; i < ui->lstFiles->count(); i++) {
        remainingFiles.append(ui->lstFiles->item(i)->text());
    }
    playList->setFiles(remainingFiles);
}
void IRiverPla::on_actionGenerate_triggered()
{
    qDebug() << "IRiverPla::on_actionGenerate_triggered()";
    try {
        playList->playlistName = ui->edtPlaylistName->text();
        playList->doWork();
    }
    catch (...) {
        qDebug() << "Playlist generation failed";
    }
}
void IRiverPla::on_actionShow_Log_triggered()
{
    qDebug() << "IRiverPla::on_actionShow_Log_triggered()";
    if (ui->edtLog->isVisible())
        ui->edtLog->hide();
    else
        ui->edtLog->show();
}


// Controls event handlers...

void IRiverPla::on_btnAdd_clicked() {
    on_action_Add_to_playlist_triggered();
}
void IRiverPla::on_btnDestination_clicked() {
    on_actionMusic_destination_triggered();
}
void IRiverPla::on_btnGenerate_clicked() {
    on_actionGenerate_triggered();
}
void IRiverPla::on_btnPlaylistdestination_clicked() {
    on_actionPlaylist_Destination_triggered();
}
/**
 * @brief IRiverPla::on_btnUp_clicked
 */
void IRiverPla::on_btnUp_clicked() {
    repositionItems(Qt::AscendingOrder);
}
/**
 * @brief IRiverPla::on_btnDown_clicked
 */
void IRiverPla::on_btnDown_clicked() {
    repositionItems(Qt::DescendingOrder);
}
void IRiverPla::on_cbKeepFolder_toggled(bool checked) {
    playList->preserveSongFolder = checked;
}
void IRiverPla::on_btnRemove_clicked() {
    on_actionRemove_triggered();
}
void IRiverPla::dropEvent(QDropEvent *event) {
    qDebug() << "IRiverPla::dropEvent()";
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QStringList droppedFiles;
            QUrl url;
            foreach (url,urls) {
                droppedFiles.append(url.toLocalFile());
            }
            addFilesToPlaylist(droppedFiles);
        }
        event->acceptProposedAction();
        ui->lstFiles->dropEvent(event);
    }
}
void IRiverPla::dragEnterEvent(QDragEnterEvent *event) {
    qDebug() << "IRiverPla::dragEnterEvent()";
    if (event->mimeData()->hasUrls())
             event->acceptProposedAction();
}




// slots for external signals...

void IRiverPla::playListError(QString time, QString category, QString message)
{
    ui->edtLog->append((QString("%1 - %2: %3").arg(time).arg(category).arg(message)));
}
void IRiverPla::playListReady()
{
    QMessageBox *mbox = new QMessageBox;
    mbox->setWindowTitle(tr("Ready"));
    mbox->setText(tr("Playlist generated"));
    mbox->show();
    QTimer::singleShot(2000, mbox, SLOT(hide()));
}



// other methods...

void IRiverPla::addFilesToPlaylist(QStringList files)
{
    try
    {
        QStringList uniqueFiles;
        // File type check -> unsynch situation with list of files in GUI vs playList object
        files = playList->filterSupportedFiles(files);
        for (int i=0; i<files.count(); i++) {
            if (ui->lstFiles->findItems(files.at(i), Qt::MatchExactly).count()!=0)
                continue;
            uniqueFiles.append(files.at(i));
        }
        ui->edtLog->append(QString("Files received: %1\r\n").arg(uniqueFiles.size()));
        ui->lstFiles->addItems(uniqueFiles);
        playList->addFiles(uniqueFiles);
        ui->edtLog->append(playList->getPLAAsString());
    }
    catch (...) {
        ui->edtLog->append("Error - IRiverPla::addFilesToPlaylist");
    }
}
/**
 * In this version we take away all items from list and sort them as we wish and then insert them back to list.
 * In next version we should create a model that takes care of sorting and then use normal listview component to show model.
 * Algorithm:
 * - Loop through each 'selected item' one by one and
 * - put it to correct position in list
 * 3rd temporary list is used as a storage for final positions. It is initially filled with listbox content.
 * Looks like that selected items are in the order that they have been selected, so we have to reorganize them
 * to be in the same order that they exist in list.
 *
 * Here Qt::AscendingOrder means that an item should bubble to top direction (towards item at 0, top of list)
 * Here Qt::DescendingOrder means that an item should be pushed downwards (towards item at count(), end of list)
 */
void IRiverPla::repositionItems(Qt::SortOrder direction)
{
    try {
        // Here we do not actually care about selected items row position in list...we assume each string is unique in list
        QList<QListWidgetItem*> selectedItems = ui->lstFiles->selectedItems();
        if (0 == selectedItems.count())
            return;
        QMap<int, QString> tmp;
        for (int i=0; i<selectedItems.count(); i++) {
            tmp[ui->lstFiles->row(selectedItems.at(i))] = selectedItems.at(i)->text();
        }
        QStringList itemsToMove;
        QMap<int,QString>::const_iterator it = tmp.constBegin();
        qDebug() << "Items to move:";
        while (it != tmp.constEnd()) {
            qDebug() << " row " << it.key() << " value " << it.value();
             itemsToMove.append(it.value());
             ++it;
         }
        //qDebug() << "SortOrder " << direction << ": " << "Selected count " << itemsToMove.count() << "; List count " << ui->lstFiles->count();
        QStringList new_order;
        for (int i=0; i<ui->lstFiles->count(); i++) {
            new_order.append(ui->lstFiles->item(i)->text());
        }
        if (direction == Qt::AscendingOrder) {
            for (int i=0; i<itemsToMove.count(); i++) {
                for (int j=0; j<new_order.count(); j++) {
                    if (itemsToMove[i] == new_order[j]) {
                        // bubble upwards item to correct place by swapping positions...
                        if (j==0) break; // beginning of list, no need to reposition...
                        qDebug() << " swap " << new_order[j] << " <-> " << new_order[j-1];
                        new_order.swap(j, j-1);
                        break; // go to next selected item...
                    }
                }
            }
        }
        else { //direction == Qt::DescendingOrder
            for (int i=itemsToMove.count()-1; i>=0; i--) {
                for (int j=new_order.count()-1; j>=0; j--) {
                    if (itemsToMove[i] == new_order[j]) {
                        // bubble item down to correct place by swapping positions...
                        if (j==new_order.count()-1) break; // last, no need to reposition...
                        qDebug() << " swap " << new_order[j] << " <-> " << new_order[j+1];
                        new_order.swap(j, j+1);
                        break;
                    }
                }
            }
        }
        ui->lstFiles->clear();
        //qDebug() << "Sorted list count " << new_order.count();
        ui->lstFiles->addItems(new_order);
        playList->setFiles(new_order);
        //qDebug() << "Sorted, adding items again to list, list count " << ui->lstFiles->count();
        qDebug() << "Done...";
    }
    catch (...) {
        qDebug("ERROR: %i",direction);
    }
}
