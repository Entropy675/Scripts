#!/bin/bash

str="$1" # search value
path="$2" # path we are searching in
tofile="$3" # location of text file to save to (or input stream...)

if [ -z "$str" ] || [ "$str" == "help" ]; then
	echo "To use this scan script, call the script with the command:"
	echo "    ./ScanFileSystem {search word} {location to search} {Location of text file to save to}"
	echo "{search word}: The word you want to search for. Case ignored for convienience."
	echo "{location to search}: ex. C:/ for your entire drive, or a specific path for all subdirectories of that folder."
	echo "{Location of text file to save to}: Default is C:/ drives current users desktop, probably put your own text file path here."
	exit 0
fi
if [ -z "$tofile" ]; then # [ -z {string} ] checks if string empty
    tofile="C:/Users/$USERNAME/Desktop/found_files.txt" # assuming some windows things (lazy)
fi

c=0
find "$path" -type d -print0 | while read -d $'\0' file; do
	((c++))
	if ((c % 50 == 0)); then #offset 50 to avoid lag from spam
		echo "Ping... $file"
		c=0
	fi
	if echo "$file" | grep -q -i "$str"; then
		echo "FOUND: $file"
		echo "$file" >> "$tofile"
	fi
done

echo "Done. Above are the lines matching $1 within $2"
echo "Stored in file $3"