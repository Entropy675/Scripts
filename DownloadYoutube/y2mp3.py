from pytube import YouTube
import sys

def download_video(url):
    try:
        # Create a YouTube object with the provided URL
        video = YouTube(url)

        # Select the highest resolution available
        stream = video.streams.get_highest_resolution()

        # Start the download
        print(f"Downloading: {video.title}")
        stream.download()
        print("Download complete!")

    except Exception as e:
        print(f"Error: {str(e)}")

if len(sys.argv) > 1:
    download_video(sys.argv[1])
else:
    print("Please provide a YouTube video URL as a command-line argument.")