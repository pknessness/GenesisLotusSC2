from os import path, environ
from typing import Union

import requests
#from loguru import logger
import os
from datetime import datetime
import shutil

#CONFIG_FILE: str = "config.yml"
MY_BOT_NAME: str = "GenesisLotus"
ZIPFILE_NAME: str = "bot.zip"

TOKEN: str = "0887d0d31ba8ed9de040f430f3018c5ec5314a62"#environ.get(API_TOKEN_ENV)
BOT_ID: str = "876"
URL: str = f"https://aiarena.net/api/bots/{BOT_ID}/"


def get_bot_description() -> str:
    return (
        f"# Genesis Lotus" "\nMade from scratch in C++"
    )

if __name__ == "__main__":

    # datetime object containing current date and time
    now = datetime.now()
    print("now =", now)

    # dd/mm/YY H:M:S
    dt_string = now.strftime("%d_%m_%Y-%H_%M_%S")

    newpath = r'builds/Bot' + dt_string
    if not os.path.exists(newpath):
        os.makedirs(newpath)
        os.makedirs(newpath + "/data")

    shutil.copyfile("build/bin/BlankBot", newpath + "/GenesisLotus")

    shutil.make_archive(newpath, 'zip', newpath)
    
    ZIPFILE_NAME = newpath + ".zip"
    
    print("Uploading bot")
    with open(ZIPFILE_NAME, "rb") as bot_zip:
        request_headers = {
            "Authorization": f"Token {TOKEN}",
        }
        request_data = {
            "bot_zip_publicly_downloadable": True,
            "bot_data_publicly_downloadable": True,
            "bot_data_enabled": True,
            "wiki_article_content": get_bot_description(),
        }
        request_files = {
            "bot_zip": bot_zip,
        }
        print(URL)
        response = requests.patch(
            URL, headers=request_headers, data=request_data, files=request_files
        )
        print(response)
        print(response.content)
        