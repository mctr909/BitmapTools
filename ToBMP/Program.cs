using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

namespace ToBMP {
    class Program {
        static void Main(string[] args) {
            for (int i = 0; i < args.Length; i++) {
                var path = args[i];
                var bmpSrc = new Bitmap(path);
                if (bmpSrc.PixelFormat == PixelFormat.Format24bppRgb) {
                    continue;
                }
                var bmpDst = new Bitmap(((bmpSrc.Width + 3) >> 2) << 2, bmpSrc.Height, PixelFormat.Format24bppRgb);
                var g = Graphics.FromImage(bmpDst);
                g.Clear(Color.White);
                g.DrawImage(bmpSrc, 0, 0);
                path = Path.GetDirectoryName(path)
                    + "\\" + Path.GetFileNameWithoutExtension(path)
                    + (Path.GetExtension(path) == ".bmp" ? "_24bit.bmp" : ".bmp");
                System.Console.WriteLine(path);
                bmpDst.Save(path, ImageFormat.Bmp);
            }
        }
    }
}
