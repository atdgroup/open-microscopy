using System;
using System.Drawing;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using FreeImageAPI;

namespace FreeImageIcs
{	
	public class IcsFile : IDisposable
	{
        private bool disposed;
        private string filepath;
        private IntPtr ics = IntPtr.Zero;

		public IcsFile(string filepath)
        {
            if (!IcsFile.IsIcsFile(filepath)) 
			    throw new ArgumentException("Not an Ics File");

            this.filepath = filepath;

            FreeImageIcsNativeMethods.IcsOpen(ref this.ics, filepath, "rw");    
        }

        public static bool IsIcsFile(string filepath)
        {
            return (FreeImageIcsNativeMethods.IsIcsFile(filepath) > 0);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                // dispose of the unmanaged resources
                if (this.ics != IntPtr.Zero)
                {
                    // If there is history write that.
                    //this.WriteHistory(this.ics);

                    // Dispose of the unmanaged resources   
                    FreeImageIcsNativeMethods.IcsClose(this.ics);
                }

                this.disposed = true;
            }
        }

        public void Dispose()
        {
            Dispose(true);

            // Tell the GC that the Finalize process no longer needs
            // to be run for this object.
            GC.SuppressFinalize(this);
        }

        ~IcsFile()
		{
			// call Dispose with false.  Since we're in the
			// destructor call, the managed resources will be
			// disposed of anyways.
			Dispose(false);	
		}

        public void Close()
        {
            this.Dispose();
        }

        public FreeImageAlgorithmsBitmap FreeImageAlgorithmsBitmap
        {
            get
            {
                FIBITMAP dib = FIBITMAP.Zero;
                
                if(FreeImageIcsNativeMethods.NumberOfDimensions(ics) == 3)
                    dib = FreeImageIcsNativeMethods.LoadFIBFromColourIcsFile(this.ics);
                else
                    dib = FreeImageIcsNativeMethods.LoadFIBFromIcsFile(this.ics);

                return new FreeImageAlgorithmsBitmap(dib);
            }
        }

        public static void SaveToFile(FreeImageBitmap fib, string filepath, bool save_metadata)
        {
            if (!FreeImageIcsNativeMethods.SaveImage(fib.Dib, filepath, save_metadata))
            {
                throw new FreeImageException("Unable to save file");
            }
        }
 
        public void AppendHistory(string key, string value)
        {
            FreeImageIcsNativeMethods.IcsAddHistoryString(this.ics, key, value);
        }

        public void AppendHistory(Dictionary<string, string> metadata)
        {
            foreach (KeyValuePair<String, String> entry in metadata)
            {
                FreeImageIcsNativeMethods.IcsAddHistoryString(this.ics, entry.Key, entry.Value);
            }
        }

        public void DeleteHistory()
        {
            FreeImageIcsNativeMethods.IcsDeleteHistory(this.ics, "");
        }

        public void SetNativeScale(int dimension, double origin, double scale, string units)
        {
            FreeImageIcsNativeMethods.SetNativeScale(this.ics, dimension, origin, scale, units);
        }

        public void SetNativeIntensityScale(double origin, double scale, string units)
        {
            FreeImageIcsNativeMethods.SetNativeIntensityScale(this.ics, origin, scale, units);
        }
    }
}
