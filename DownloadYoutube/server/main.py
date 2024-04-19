from flask import Flask, render_template, request
from pytube import YouTube
import logging
import threading
import shutil
import time
import os

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')

logger = logging.getLogger()

DELETE_INTERVAL = 60
EXPIRY_TIME = 60 * 5
DOWNLOAD_DIR = os.path.join(os.path.dirname(__file__), 'static')


app = Flask(__name__)

def download_video(url):
    try:
        video = YouTube(url)

        stream = video.streams.get_highest_resolution()

        if not os.path.exists(DOWNLOAD_DIR):
            os.makedirs(DOWNLOAD_DIR)

        logging.info(f"Downloading: {video.title}")
        filename = os.path.join(DOWNLOAD_DIR, video.title + ".mp4")
        stream.download(output_path=DOWNLOAD_DIR)
        return filename

    except Exception as e:
        logging.error(f"Error downloading video: {str(e)}")
        return None


@app.route('/')
def index():
    return render_template('index.html')

@app.route('/download', methods=['POST'])
def download():
    url = request.form['url']
    print(url)
    filename = download_video(url)
    if filename is not None:
        return render_template('download.html', filename=os.path.basename(filename))
    else:
        return render_template('error.html', error="Error downloading the video. Please check the provided URL.")

def delete_old_videos():
    current_time = time.time()
    expiry_time = current_time - EXPIRY_TIME  # 300 seconds = 5 minutes
    
    logging.info(f"Cleaning downloaded videos...")
    for file_name in os.listdir(DOWNLOAD_DIR):
        file_path = os.path.join(DOWNLOAD_DIR, file_name)
        if os.path.isfile(file_path):
            modification_time = os.path.getmtime(file_path)
            if modification_time < expiry_time:
                try:
                    os.remove(file_path)
                    print(f"Deleted old file: {file_path}")
                except OSError as e:
                    print(f"Error deleting file: {file_path}, {e}")
    

if __name__ == '__main__':
    threading.Timer(DELETE_INTERVAL, delete_old_videos).start()
    app.run(debug=True)
