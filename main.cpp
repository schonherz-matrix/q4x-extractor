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

    // Ezt is a metaadatok között kiírni buzis...
    f.read(2); //w a sch szelessege pixelben
    f.read(2); //h a sch magassaga pixelben

    qStdOut() << "\tExtacting qp4 from qpx\n";
    if (! extr_int(f, i.baseName()+".qp4"))
    {}

    qStdOut() << "\tExtacting qpr from qpx\n";
    if (! extr_int(f, i.baseName()+".qpr"))
    {}

    if(f.bytesAvailable() >=4) {
        qStdOut() << "Exctracting mp3 from qpx\n";

        quint32 size;
        f.read(reinterpret_cast<char*>(&size), 4);
        size = qFromBigEndian<quint32>(size);
        QFile audio(i.baseName()+".mp3");
        audio.open(QIODevice::WriteOnly);
        audio.write(f.read(size));
    }
    return true;
}

bool extr_int(QFile &input, const QString &outputfile)
{
    quint32 size;
    input.read(reinterpret_cast<char*>(&size), 4);
    size = qFromBigEndian<quint32>(size);
    quint32 expsize = size*9; // kbkb...
    expsize = qToBigEndian<quint32>(expsize);
    QByteArray compressed = input.read(size);
    compressed.prepend(reinterpret_cast<const char*>(&expsize), 4);
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
