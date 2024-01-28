#!/bin/ash

# Exit script on any command that returns a non-zero status
set -e

timestamp() 
{
    date +"%Y-%m-%d-[%T]"
}

# log dir path
logdir="./updatelogs"

# make the log directory if it doesn't exist
mkdir -p "$logdir"

# Set the maximum number of files you want to keep
maxLogs=30 # month should be good

# Log file for the script
LOG_FILE="$logdir/$(timestamp)_package_update.log"

# Redirect all output to the log file
exec >> "${LOG_FILE}" 2>&1

# Check if opkg is available (OpenWrt system)
if command -v opkg &> /dev/null; then
    package_manager="opkg"
elif command -v pip &> /dev/null; then
    package_manager="pip"
else
    echo "Error: Neither opkg nor pip found. Exiting."
    exit 1
fi

# Print start timestamp
echo "========== Package Update Script Started: $(timestamp) =========="

if [ "$package_manager" = "opkg" ]; then
	# Update lists
	opkg update

	# Upgrade all installed packages
	upgradeable_packages=$(opkg list-upgradable | cut -f 1 -d ' ')

	if [ -n "$upgradeable_packages" ]; then
		echo "==> Upgrading packages: $upgradeable_packages"
		echo "$upgradeable_packages" | xargs opkg upgrade
	else
		echo "-> No upgradable packages to update."
	fi
    # Remove packages marked obsolete...
	obsolete_packages=$(opkg list-installed | awk '/^\*/ {print $2}')

	if [ -n "$obsolete_packages" ]; then
		opkg remove "$obsolete_packages"
		echo "==> Obsolete packages removed: $obsolete_packages"
	else
		echo "-> No obsolete packages to remove."
	fi

elif [ "$package_manager" = "pip" ]; then
    # Upgrade all Python packages
    pip install --upgrade -r <(pip freeze)
fi

echo "Removing old logs (for storage reasons)...."

# Change to the specified directory
cd "$logdir" || exit 1

# Count the number of files in the directory
logCount=$(ls -1 | wc -l)

# Check if the file count exceeds the maximum limit
if [ "$logCount" -gt "$maxLogs" ]; then
    # List files, sort by modification time, and delete the oldest file
    oldestLog=$(ls -t | tail -n 1)
    rm "$oldestLog"
    echo "==> Deleted the oldest log file: $oldestLog"
else
    echo "-> No logs to delete. Current log count: $logCount"
fi

echo "========== Package Update Script Completed: $(timestamp) =========="
