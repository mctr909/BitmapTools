using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

namespace ToBMP {
    class Program {
        static void Main(string[] args) {
            for (int i = 0; i < args.Length; i++) {
                var path = args[i];
                if (".bmp" == Path.GetExtension(path)) {
                    continue;
                }
                var bmpSrc = new Bitmap(path);
                var bmpDst = new Bitmap(((bmpSrc.Width + 3) >> 2) << 2, bmpSrc.Height);
                var g = Graphics.FromImage(bmpDst);
                g.DrawImage(bmpSrc, 0, 0);
                path = Path.GetDirectoryName(path)
                    + "\\" + Path.GetFileNameWithoutExtension(path)
                    + ".bmp";
                System.Console.WriteLine(path);
                bmpDst.Save(path, ImageFormat.Bmp);
                bmpSrc.Dispose();
                bmpDst.Dispose();
            }
        }
    }
}
