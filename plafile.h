#ifndef PLAFILE_H
#define PLAFILE_H

#include <QObject>
#include <QString>
#include <QStringList>

class QFileInfo;

/**
 * \brief plaFile implements the file support itself, it understands the structure of PLA format.
 * \remarks Found PLA 'spec' copied here below (http://phintsan.kapsi.fi/iriver-t50.html)
 *
 * Playlist file consists of 1+N null-padded 512-byte frames, where N is the number of songs in the playlist.
 * The first frame is a header, and the rest are song frames. The header starts with a 32-bit big-endian integer
 * specifying N. Immediately after that there is an ASCII string 'iriver UMS PLA', and that's all for the header frame.
 *
 * Each song frame begins with a 16-bit big-endian integer specifying the index of the first non-directory character
 * of the song's full filename. This index is one-based. The index is there probably just to help the player to strip
 * the directory part out of the song's filename, if there is no title tag in the song file. Immediately after the
 * index comes the song's null-terminated full filename. As the filesystem type is VFAT, it is encoded as big-endian
 * UTF-16 without a byte order mark. Absolute paths used. The index and filename are all there is in a single song frame.
 * Note that the filename must fit into one 512-byte frame. So the filename, including the directory part, can have at
 * most 255 (two-byte) characters. The player-accompanied iriver plus program seems to mess up your data without warning
 * if you happen to use more than 255 characters, so don't do that.
 *
 * Ex. Header frame (512 byte array)
 * [0000 0000] 32 bit, Number of songs in the playlist
 * [...] 'iriver UMS PLA'
 * [...] rest of the header 512byte area is 0
 *
 * Ex. Song frame (512 byte array)
 * [0000] 16 bit, 1 based index position number that specifies where the actual song filename starts in the path
 * [...] UTF-16 absolute, full file path (null terminated), make sure 512 bytes is not exceeded for each song!
 *
 * Program usage
 *
 * \li Select the files you would like to have in destination
 * \li Order the files to desired order
 * \li Define the destination location (playlist file in same place as files
 * \li Generate playlist
 *
 * Generating playlist will check that we have access to destination and it also
 * checks which files exists there already and which needs to be copied there.
 * File copying will be done first and after successfully copied all necessary files
 * to destination the playlist is copied there also.
 */
class plaPlayList : public QObject
{
    Q_OBJECT
private:
    // Private variables and methods
    QStringList m_lstSrcFiles;
    QStringList m_lstDstFiles;

    bool checkDestinationAvailability();
    bool checkDestinationFilesAvailability(QStringList &duplicateFiles);
    bool copyMissingFilesToDestination();
    bool getFileName(QString song, QString*outFile, qint16*);
    bool checkIfFilesExists(QStringList *alreadyExists);
    bool checkIfIEnoughCapacity(int* deviceTotal, int* neededSize);
    bool generatePLAFile();

public:
    // Public interface
    explicit plaPlayList(QObject *parent = 0);

    int addFile(QString name);
    int addFiles(QStringList names);
    int addDirectory(QString name);
    int setFiles(QStringList names);

    bool doWork();
    QString getPLAAsString();

    QString getFileFilter();
    QStringList filterSupportedFiles(QStringList);

    long playlistFileAmount();
    long plaContentSize();

    QString musicFileDestination;
    QString playlistDestination;
    QString playlistName;
    bool preserveSongFolder;
    const QString iriverText = "iriver UMS PLA"; /**<  constant text to be written to header part of PLA */

    /** Struct defines supported file types this program supports. */
    struct {
        QString ogg = "ogg";
        QString mp3 = "mp3";
        QString wma = "wma";
        QString flac = "flac";
    } plaSupportedFiles;

signals:
    void OnError(QString time, QString category, QString message);          /**< Notifies errors that has happened */
    void OnFileCopied(QString time, QString category, QString fileName);    /**< Notifies a successfull file copy to destination */
    void OnReady();                                                         /**< Notifies that playlist operations finished to destination */
};

#endif // PLAFILE_H
