import subprocess
from os import system
from time import sleep
import argparse
import multiprocessing
import sys
from datetime import datetime
import google.auth
from google.oauth2 import service_account
from googleapiclient.discovery import build
import os

SERVICE_ACCOUNT_FILE = 'CREDENTIALS.json'

def startProcess(path, fileName):
    commands = [path, fileName]
    print("Starting process")
    subprocess.run(commands)

def upload_file(file_path):
    credentials = service_account.Credentials.from_service_account_file(
        SERVICE_ACCOUNT_FILE,
        scopes=['https://www.googleapis.com/auth/drive']
    )
    
    drive_service = build('drive', 'v3', credentials=credentials)

    file_metadata = {
        'name': os.path.basename(file_path)
    }

    media = drive_service.files().create(
        body=file_metadata,
        media_body=file_path,
        fields='id'
    ).execute()
        
    file_id = media.get('id')
    
    permission = {
        'type': 'user',
        'role': "writer",
        'emailAddress': "quiteweirdadress@gmail.com",
    }
        
    drive_service.permissions().create(fileId=file_id, body=permission).execute()

def main():
    parser = argparse.ArgumentParser(description="")

    parser.add_argument(
            '--path',
            type=str,
            help = "",
            default = "./Datagen"
    )

    parser.add_argument(
            '--threads',
            type=int,
            help = "",
            default = multiprocessing.cpu_count()
    )

    args = parser.parse_args()

    batch = 0

    while True:
        batch += 1
        
        fileList = []

        for i in range(0, args.threads):
            fileName = str(datetime.today().strftime('%Y%m%d')) + "_thread" + str(i) +"_batch" + str(batch) + ".bin"
            fileList.append(fileName)
            thread = multiprocessing.Process(target = startProcess, args = (args.path, fileName))
            thread.start()
            sleep(1)

        sleep(3600)

        commands = ['killall', "Datagen"]
        process = subprocess.Popen(commands)
        process.wait()
        
        for f in fileList:
            upload_file(f)
            os.remove(f)

    

if __name__ == "__main__":
    main()
