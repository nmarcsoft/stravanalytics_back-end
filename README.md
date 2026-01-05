# STRAVA

## Why use this ?

This application extract data from the strava API and can be used to trace performance.

You can apply filer and adjust your request simply from the command.json file.

## How to use it ?

### API Connection

First, you need to connect to strava and go on this link :
[https://www.strava.com/settings/api](https://www.strava.com/settings/api)
and copy/paste your Cliend_ID and Client_SECRET in the file var.csv.

### Build the project

to build it, you need : cmake and nodejs.
```bash
sudo pacman -S cmake
sudo pacman -S nodejs-lts-jod
```

at the root of the project open 2 terminal, on the first one, run the back-end :
```
cmake -B build
make -C build
./build/strava
```
On the second one, the front-end :
```
cd stravanalytics/
npm run dev
```


### Command to send

The command.json is quite simple, can put value in the pre-registered field,
and it will automatically adjust your output
This is a sample :

```json
{
    "activities" : {
        "id" : "",
        "comments" : "",
        "kudos": "",
        "laps": "",
        "contient": ""
    },

    "filtre" : {
        "nom" : ["Test", "Hello"],
        "distance" : [-1, -1],
        "duree": [-1, -1],
        "d+" : [-1, 100],
        "date": ["01/01/2025", "31/12/2025"],
    }
}
```

It will extract all running activity, from 1
januray 2025 to the 31 decembre 2025 containing Test or Hello in the title.

### Result

Once you ran the backend and the frontend (/stravanalitics), go to your localhost port 5173 and track your prograssion on various training.

https://github.com/user-attachments/assets/f9f4f91a-54d4-4a3b-9af4-458d3c45ff45



## Utility of the project

This project is for educational purpose, I wanted to learn C++,
and the strava API is free, so we can make cool program pretty
esealy with this API.
This will also be usefull
to track my progress for my goal of sub 42' on 10km !
