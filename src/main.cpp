#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

static bool extractFile(QDataStream& in, QString outputFile) {
  quint32 size;
  in >> size;

  QByteArray buffer;
  // header + size
  // https://doc.qt.io/qt-5/qbytearray.html#qUncompress
  buffer.reserve(4 + size);
  QDataStream bufferStream(&buffer, QIODevice::WriteOnly);

  // Expected size as Big-endian
  bufferStream << size * 9;
  buffer.append(in.device()->read(size));

  QFile output(outputFile);
  if (!output.open(QFile::WriteOnly)) return false;

  auto uncompressed = qUncompress(buffer);
  if (uncompressed.size() == 0) return false;
  output.write(uncompressed);

  return true;
}

static bool extract(QString filename) {
  QFile file(filename);
  QFileInfo fileInfo(filename);
  if (!file.open(QFile::ReadOnly)) return false;

  QDir dir;
  dir.mkdir(fileInfo.baseName());
  dir.setCurrent(fileInfo.baseName());

  QDataStream in(&file);
  auto magic = file.read(4);
  if ("Q4X1" != magic && "Q4X2" != magic) return false;

  // Skip width, height values
  file.seek(file.pos() + 4);

  qInfo() << "\tExtracting qp4";
  if (!extractFile(in, fileInfo.baseName() + ".qp4")) return false;
  qInfo() << "\tExtracting qpr";
  if (!extractFile(in, fileInfo.baseName() + ".qpr")) return false;

  quint32 audioSize;
  in >> audioSize;
  if (audioSize > 0) {
    auto audioMagic = file.peek(4);

    // Correct audio type is checked when exporting animation
    // No need to check complicated mp3 header
    QString extension = ("OggS" == audioMagic) ? "ogg" : "mp3";

    qInfo().noquote() << "\tExtracting" << extension;
    QFile output(fileInfo.baseName() + "." + extension);
    if (!output.open(QFile::WriteOnly)) return false;
    output.write(file.read(audioSize));
  }

  return true;
}

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);
  QCoreApplication::setApplicationName("q4x-extractor");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "Extracts qp4, qpr, mp3/ogg(if exists) files from q4x to a separate "
      "folder in the current working directory");
  parser.addHelpOption();
  parser.addVersionOption();

  parser.addPositionalArgument(
      "path", QCoreApplication::translate(
                  "path", "List of directories or q4x files to extract from"));

  parser.process(a);

  auto currentDir = QDir::currentPath();
  for (auto path : parser.positionalArguments()) {
    QFileInfo info(path);

    if (info.isDir()) {
      qInfo().noquote() << "Extracting files from directory" << path;
      QDirIterator it(path, QStringList("*.q4x"),
                      QDir::Files | QDir::NoDotAndDotDot,
                      QDirIterator::Subdirectories);
      while (it.hasNext()) {
        auto file = it.next();
        qInfo().noquote() << "Extracting file" << file;
        if (!extract(file)) qInfo() << "\tExtracting failed";

        QDir::setCurrent(currentDir);
      }
    } else {
      qInfo().noquote() << "Extracting file" << path;
      if (!extract(path)) qInfo() << "\tExtracting failed";

      QDir::setCurrent(currentDir);
    }
  }

  return 0;
}
