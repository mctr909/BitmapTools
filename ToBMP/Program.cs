using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

namespace ToBMP {
    class Program {
        static void Main(string[] args) {
            for (int i = 0; i < args.Length; i++) {
                var path = args[i];
                var bmpSrc = new Bitmap(path);
                if (Path.GetExtension(path) == ".bmp" && bmpSrc.PixelFormat == PixelFormat.Format24bppRgb) {
                    continue;
                }
                var bmpDst = new Bitmap(bmpSrc.Width, bmpSrc.Height, PixelFormat.Format24bppRgb);
                var g = Graphics.FromImage(bmpDst);
                g.Clear(Color.White);
                g.DrawImage(bmpSrc, 0, 0, bmpSrc.Width, bmpSrc.Height);
                path = Path.GetDirectoryName(path)
                    + "\\" + Path.GetFileNameWithoutExtension(path)
                    + (Path.GetExtension(path) == ".bmp" ? "_24bit.bmp" : ".bmp");
                System.Console.WriteLine(path);
                bmpDst.Save(path, ImageFormat.Bmp);
            }
        }
    }
}
