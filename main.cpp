#include <QCoreApplication>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QtEndian>
#include <QRegExp>
#include <QTextStream>
#include <cstdio>
#include <limits>

QTextStream& qStdOut()
{
    static QTextStream ts( stdout );
    return ts;
}

bool extr_int (QFile&f, const QString& outputfile);

bool extract (const char* filename)
{
    QFile f(filename);
    QFileInfo i (filename);
    if (!f.open (QIODevice::ReadOnly))
    {
        qStdOut() << "\tUnable to open file\n";
        return false;
    }
    QByteArray MAGIC = f.read(4);
    if (QString("Q4X1") != MAGIC) return false;

    qStdOut() << "\tExtacting qp4 from qpx\n";
    if (! extr_int(f, i.baseName()+".qp4"))
            return false;

    if(f.bytesAvailable() > 1) {
        qStdOut() << "Exctracting mp3 from qpx\n";
        QFile audio(i.baseName()+".mp3");
        audio.open(QIODevice::WriteOnly);
        audio.write(f.readAll());
    }
    return true;
}

bool extr_int(QFile &input, const QString &outputfile)
{
    quint32 size;
    input.read(reinterpret_cast<char*>(&size), 4);
    size = qFromBigEndian<quint32>(size);
    QByteArray compressed = input.read(size);
    QByteArray uncompressed = qUncompress(compressed);
    if (uncompressed.length() == 0) return false;
    QFile output(outputfile);
    output.open(QIODevice::WriteOnly);
    qint64 writed = output.write(uncompressed);
    return writed == uncompressed.length();
}

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);

    for (int i = 1; i < argc; ++i){
        const char* filename = argv[i];
        qStdOut() << "Extracting file " << filename << '\n';
        bool success = extract (filename);
        if (!success)
            qStdOut() << "Extracting failed\n";
    }

    return 0;
    //return a.exec();

}
