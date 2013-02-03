#include "plafile.h"
#include <QApplication>
#include <QByteArray>
#include <QDateTime>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>


plaPlayList::plaPlayList(QObject *parent) :
    QObject(parent)
{
    playlistName = "playlist.pla";
    musicFileDestination = playlistDestination = QApplication::applicationDirPath();
    preserveSongFolder = true;
}
int plaPlayList::addFile(QString name)
{
    m_lstFiles.append(name);
    return m_lstFiles.count();
}
int plaPlayList::setFiles(QStringList names)
{
    m_lstFiles = names;
    return m_lstFiles.count();
}
int plaPlayList::addFiles(QStringList names)
{
    m_lstFiles += names;
    return m_lstFiles.count();
}
/**
 * @brief Extracts all files from specified directory and adds supported files to playlist
 * @param name Path to directory that will be checked
 * @return Number of files accepted from given directory
 */
int plaPlayList::addDirectory(QString name)
{
    int retVal = 0;
    QDir dir(name);
    if (!dir.exists()) {
        OnError(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"), "ERROR", QString("Directory '%1' was not found.").arg(name));
        return retVal;
    }
    QStringList acceptedFiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    acceptedFiles = filterSupportedFiles(acceptedFiles);
    addFiles(acceptedFiles);
    return acceptedFiles.count();
}
/**
 * @brief Used to get file names from the playlist.
 * @return Playlist files in one string separated by a line feed and carriage return.
 */
QString plaPlayList::getPLAAsString()
{
    return m_lstFiles.join("\r\n");
}
/**
 * @brief Returns a proper filter for 'FileOpen dialog', enumerates all supported file formats.
 * This is not used at the moment and don't know if it is good to even try to restrict input file formats.
 * @return File filter string
 */
QString plaPlayList::getFileFilter()
{
    QString retVal = "All Files (*)";
    return retVal;
}
/**
 * @brief Checks that file names will end to one of the supported formats...without actually checking the file content.
 * @param files Files that need to be checked.
 * @return QStringList Contains files that are accepted as valid format.
 */
QStringList plaPlayList::filterSupportedFiles(QStringList files)
{
    QStringList retVal;
    QString name;
    foreach (name, files) {
        if (name.endsWith(plaSupportedFiles.ogg)
         || name.endsWith(plaSupportedFiles.mp3)
         || name.endsWith(plaSupportedFiles.flac)
         || name.endsWith(plaSupportedFiles.wma))
            retVal.append(name);
    }
    qDebug() << "PlayList file anem filtering: suggested files count " << files.count() << ", accepted count " << retVal.count();
    return retVal;
}
/**
 * @brief Used to get the number of files selected to playlist.
 * @return Number of files in the playlist.
 */
long plaPlayList::playlistFileAmount()
{
    return m_lstFiles.size();
}
/**
 * @brief Used to get the amount of bytes in the generated playlist file (*.pla)
 * @return Number of bytes in PLA file
 */
long plaPlayList::plaContentSize()
{
    return 0;
}
bool plaPlayList::doWork()
{
    try {
        //
        // work list
        // 1. generate destination files list (correct path information)
        // 2. check existence of destination files and filter out already existing ones
        // 3. check that we have enough space for missing files
        // 4. copy missing files to destination
        // 5. generate playlist and copy it to 'playlist destination'
        //
        if (checkDestinationFilesAvailability())
        OnReady();
        return true;
    }
    catch (...) {
        qDebug() << "plaPlayList::generatePLAFile - Error while creating playlist";
        return false;
    }
}
bool plaPlayList::generatePLAFile()
{
    try {
        // 5. generate playlist
        qint32 fileCount = (qint32)m_lstFiles.count();
        if (playlistName.isEmpty())
            playlistName = "playlist.pla";
        if (!playlistName.endsWith(".pla"))
            playlistName.append(".pla");
        QFile file(playlistDestination + "/" + playlistName);
        file.open(QIODevice::Truncate | QIODevice::WriteOnly);
        if (!file.isOpen()) {
            qDebug() << "file does not open?";
            return false;
        }
        QDataStream out(&file);
        out.setByteOrder(QDataStream::BigEndian);

        // Header info: qint32 (4 bytes) + iriver_text (14 bytes) = 18 bytes -> rest 494 bytes = 0
        out << fileCount;
        out.writeRawData(iriverText.toLatin1(), iriverText.size()); // tolatin1 is serialized as 'length,string' ->use writeRawData
        int i = 494;
        while (0 < i) {
            out << (qint8)0;
            --i;
        }

        // File Info
        QString outputFile = "";
        qint16 nameIndex = 0;
        foreach (QString song, m_lstFiles) {
            if (song.length()>=512) {
                qDebug() << "Song " << song << " file path was too long, skipping it";
                continue;
            }
            if(!getFileName(song, &outputFile, &nameIndex)) {
                qDebug() << "Song " << song << " path and index extraction failed, PLA is not usable!";
                OnError(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"), "ERROR", QString("Song '%1' path and index extraction failed, PLA is not usable!").arg(song));
                return false;
            }
            qDebug() << " song: " << outputFile;
            out << nameIndex;
            out << (qint8)0; // ugly workaround! put '00' byte before '\' mark which is missing from outputFile string (for some reason)?
            out.writeRawData((const char*) outputFile.utf16(), outputFile.size() * 2);
            for (int i=(3+outputFile.size()*2); i<512; i++)
                out << (qint8)0;
        }
        qDebug() << "plaPlayList::generatePLAFile - data written, playlist size: " << file.size() << ", should be 512 + " << m_lstFiles.count() << "*512 = " << 512 + m_lstFiles.count() * 512;
        file.close();
        OnReady();
        return true;
    }
    catch (...) {
        qDebug() << "plaPlayList::generatePLAFile - Error while creating playlist";
        return false;
    }
}
/**
 * @brief Idea is to just check that main level (folder) exists into which music files are copied.
 * @return true if playlist files destination folder (main level) exists and false otherwise
 */
bool plaPlayList::checkPlaylistDestinationAvailability()
{
    QDir dir(playlistDestination);
    return dir.exists();
}
/**
 * @brief Idea is to:
 * \li check that main level (folder) exists into which music files are copied
 * \li check destination if that already has some files that are selected in playlist and try to remove from the
 * @return true if music files destination folder (main level) exists and false otherwise
 */
bool plaPlayList::checkDestinationFilesAvailability(QStringList &duplicateFiles)
{
    QDir dir(musicFileDestination);
    if (!dir.exists()) {
        return false;
    }
    QStringList destinationFiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList removeItems;
    for (int i=0; i < duplicateFiles.count(); i++) {
        if (destinationFiles.contains(duplicateFiles.at(i)))
            removeItems.append(duplicateFiles.at(i));
    }
    foreach (QString file, removeItems) {
        qDebug() << " removing playlist item " << file << " while it already exists in destination " << musicFileDestination;
        duplicateFiles.removeOne(file);
    }
    return true;
}
bool plaPlayList::copyMissingFilesToDestination()
{
    return true;
}
/**
 * @brief Get the actual filename and possibly folder that will be written to PLA file
 * This program manages location of songs so, that user can define the 'root' for music
 * files under which all music files and folders will be copied or referenced. Similarly
 * user can define the location for Playlist files generated by this program. iRiver
 * interprets playlist paths so that '/' references to the root of music player.
 *
 * If user has checked that 'preserve folder' then playlist references files so that
 * it adds the immediate folder where song exists beneath the 'music root' so:
 * 'music root'/'folder'/'song'. The other choice is 'music root'/'song'
 *
 * @param fi Input file to be examined
 * @param outFile Name of the file + one folder level above it if preserveSongFolder is true
 * @param nameIndex 1 based position in absolutefilepath from which the actual filename starts
 * @return true if successfully retrieved information, false otherwise
 */
bool plaPlayList::getFileName(QString song, QString *outFile, qint16 *nameIndex)
{
    QFileInfo inputFile(song);
    *outFile = "";
    *nameIndex = 0;
    if (inputFile.absoluteFilePath().isEmpty())
        return false;
    QString folderName = "";
    QString fileName = "";
    if (preserveSongFolder) {
        folderName = inputFile.absoluteDir().dirName();
    }
    if (preserveSongFolder) {
        fileName = QString("%1\\%2\\%3").arg(musicFileDestination).arg(folderName).arg(inputFile.fileName());
    }
    else {
        fileName = QString("%1\\%2").arg(musicFileDestination).arg(inputFile.fileName());
    }
    *outFile = fileName;
    *nameIndex = (qint16)outFile->indexOf(inputFile.fileName()) + 1;
    return true;
}
/**
 * @brief Method checks all playlist files whether they already exists in destination
 * @param alreadyExists List of files that can do exist already in destination
 * @return true if some files can already exists in destination, false otherwise
 */
bool plaPlayList::checkIfFilesExists(QStringList *alreadyExists)
{
    try {
        QFileInfo fi;
        for (int i=0; i<m_lstFiles.count(); i++) {
            fi.setFile(m_lstFiles.at(i));
        }
    }
    catch (...) {
    }
}
/**
 * @brief Verify that we can copy all necessary files to destination and that device has enough free diskspace.
 * @param deviceTotal device amount of free space
 * @param neededSize amount of megabytes needed to synchronize the playlist
 * @return true if playlist can be copied to destination, false otherwise (or some error happened)
 */
bool plaPlayList::checkIfIEnoughCapacity(int *deviceTotal, int *neededSize)
{
}

