# Game controller framework using hand genstures

This project proposes a framework for a game controller that is using hand gestures as input. Code languege is Python, and the code is mainly tested in Python 3.9.0, but does also work for Python 3.11.4.

The code is capable of detecting four different gestures; "Standing still", "Jump", "Kick" and "Walk", these are preformed as shown in the pictures stand_still.jpg, jump.jpg, kick.jpg and walk.jpg.

These gestures are then sent as states to an IP-adress on a port defined by the user (l19-l20) with OSC protocol.

## Requirements
Install the given requirements file by this command line:

```
pip install -r requirements.txt
```

The program may also need the following running condition to secure as accurate results as possible: a background that is separable from the user’s hand, no lighting that can interfere with the camera and having the camera in a position so that it is able to capture the whole hand, including the wrist.

## Files
Files included in this project.

### hand_landmarks.py
This is the main program of the project. The details about the program is described in the comments in this file.

### hand_landmarker.task
The Hand Landmarker requires a hand landmark model bundle to be downloaded and stored in your project directory. This is the hand landmark model bundle task-file [[MediaPipe]](https://developers.google.com/mediapipe/solutions/vision/hand_landmarker).

### data_on_gestures.xlsx
Excel sheet that consist of data from testing and tuning the system. 

### Pictures of gestures
#### jump.jpg
#### kick.jpg
#### stand_still.jpg
#### walk.jpg


## How to run
Run the command line argument:

```
python3 hand_landmarks.py
```

Stop the program by pressing the key 'q' and the program will print "Quitting" before terminating the processes.
