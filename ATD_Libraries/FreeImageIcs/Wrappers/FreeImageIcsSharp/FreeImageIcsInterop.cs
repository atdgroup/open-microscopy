using System;
using System.Text;
using System.Drawing;
using System.Runtime.InteropServices;

using FreeImageAPI;

namespace FreeImageIcs
{
	internal static class FreeImageIcsNativeMethods
	{
        // Ics
        const string FreeImageIcsDll = "FreeImageIcs.dll";

        [DllImport(FreeImageIcsDll, EntryPoint = "FreeImageIcs_IcsSetNativeIntensityScale")]
        internal static extern int SetNativeIntensityScale(IntPtr ics, double origin, double scale, string units);

        [DllImport(FreeImageIcsDll, EntryPoint = "FreeImageIcs_IcsSetNativeScale")]
        internal static extern int SetNativeScale(IntPtr ics, int dimension, double origin, double scale, string units);

        [DllImport(FreeImageIcsDll, EntryPoint = "FreeImageIcs_IcsAddHistoryString")]
        internal static extern int IcsAddHistoryString(IntPtr ics, string key, string value);

        [DllImport(FreeImageIcsDll, EntryPoint="FreeImageIcs_IcsDeleteHistory")]
        internal static extern int IcsDeleteHistory (IntPtr ics, string key);

        [DllImport(FreeImageIcsDll, EntryPoint="FreeImageIcs_IcsNewHistoryIterator")]
        internal static extern int IcsNewHistoryIterator (IntPtr ics, ref IntPtr it, string key);

        [DllImport(FreeImageIcsDll, EntryPoint="FreeImageIcs_IcsGetHistoryStringI")]
        internal static extern int IcsGetHistoryStringIterator (IntPtr ics, IntPtr it, out StringBuilder value);

        [DllImport(FreeImageIcsDll, EntryPoint = "FreeImageIcs_IcsOpen")]
        internal static extern int IcsOpen(ref IntPtr ics, string filePath, string mode);

        [DllImport(FreeImageIcsDll, EntryPoint = "FreeImageIcs_IcsClose")]
        internal static extern int IcsClose(IntPtr ics);

		[DllImport(FreeImageIcsDll, EntryPoint="FreeImageIcs_IsIcsFile")]
		internal static extern int IsIcsFile (string filepath);

		[DllImport(FreeImageIcsDll, EntryPoint="FreeImageIcs_LoadFIBFromIcsFilePath")]
        internal static extern FIBITMAP LoadFIBFromIcsFilePath(string filepath, bool padded);

        [DllImport(FreeImageIcsDll, EntryPoint="FreeImageIcs_LoadFIBFromIcsFile")]
        internal static extern FIBITMAP LoadFIBFromIcsFile(IntPtr ics);

        [DllImport(FreeImageIcsDll, EntryPoint = "FreeImageIcs_NumberOfDimensions")]
        internal static extern int NumberOfDimensions(IntPtr ics);

        [DllImport(FreeImageIcsDll, EntryPoint = "FreeImageIcs_LoadFIBFromColourIcsFile")]
        internal static extern FIBITMAP LoadFIBFromColourIcsFile(IntPtr ics);

        [DllImport(FreeImageIcsDll, EntryPoint = "FreeImageIcs_SaveImage")]
        internal static extern bool SaveImage(FIBITMAP fib, string filepath, bool save_metadata);

        [DllImport(FreeImageIcsDll, EntryPoint="FreeImageIcs_GetFirstIcsHistoryValueWithKey")]
        internal static extern bool GetFirstIcsHistoryValueWithKey(IntPtr ics, string key, string value);

        [DllImport(FreeImageIcsDll, EntryPoint="FreeImageIcs_ReplaceIcsHistoryValueForKey")]
        internal static extern bool ReplaceIcsHistoryValueForKey(IntPtr ics, string key, string value);

        [DllImport(FreeImageIcsDll, EntryPoint = "IsIcsHistoryIteratorValid")]
        internal static extern bool IsIcsHistoryIteratorValid(IntPtr ics, IntPtr it);

        [DllImport(FreeImageIcsDll, EntryPoint = "JoinKeyValueIntoHistoryString")]
        internal static extern bool JoinKeyValueIntoHistoryString(out StringBuilder joinedString, string key, string value);
	}
}
