import sys
import os
import moviepy.editor as mp

def convert_mp4_to_mp3(mp4_file, mp3_file):
    video = mp.VideoFileClip(mp4_file)
    video.audio.write_audiofile(mp3_file)


# Check if the correct number of command-line arguments is provided
if len(sys.argv) != 2:
    print("Usage: python convertMp4Mp3.py [mp4_file_path]")
    sys.exit(1)

mp4_file_path = sys.argv[1]
mp4_file_name = os.path.basename(mp4_file_path)
mp3_file_path = os.path.splitext(mp4_file_name)[0] + ".mp3"

convert_mp4_to_mp3(mp4_file_path, mp3_file_path)
