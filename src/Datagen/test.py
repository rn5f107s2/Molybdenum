import subprocess
from os import system
from time import sleep
import argparse
import multiprocessing
import sys


def startProcess(path, threadId):
    commands = [path, str(threadId)]
    print("Starting process ", threadId)
    subprocess.run(commands)

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

    args = parser.parse_args();
    processList = []

    for i in range(0, args.threads):
        thread = multiprocessing.Process(target = startProcess, args = (args.path, i,  ))
        thread.start();
        processList.append(thread)
        sleep(1)

    for line in sys.stdin:
        break

    for p in processList:
        p.terminate()
        print("-1")
    

if __name__ == "__main__":
    main()
