Run with:
chmod +x ScanFileSystem.sh             # Allow Executable Permissions
./ScanFileSystem.sh {$1} {$2} {$3}     # RUN

{$1} = Search term that you are looking for in file name (can't be empty, ignores case)
{$2} = Directory that you are searching inside of. Can do C:/, but I recommend specifying something for speed.
{$3} = Location to save to. Default is C:/Users/$USER/Desktop/found_files.txt if that works for you, you can leave it empty.