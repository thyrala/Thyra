import csv
import cv2 as cv
import mediapipe as mp # Google framework
import numpy as np
import os
import time

from mediapipe.tasks import python
from mediapipe.tasks.python import vision
from pythonosc.udp_client import SimpleUDPClient

ret = None

def run_handlandmarks(video_source, filename=None, imagename=None):
    """
    Function for running the program
    """
    # IP-adress and port used for OSC messaging, user-defined
    ip = "127.0.0.1" # localhost 
    port = 8000

    #Creating UDP client 
    client = SimpleUDPClient(ip, port)

    # Using the os-module to create the right path for the modle bundle file no matter which PC that is running the program
    model_path = os.path.abspath('hand_landmarker.task')

    # Opening video source with OpenCV
    cap = cv.VideoCapture(video_source)
    cap.set(cv.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv.CAP_PROP_FRAME_HEIGHT, 480)
    fps = float(cap.get(cv.CAP_PROP_FPS))

    if cap.isOpened():
        print(f"Opened video source {video_source}")
    else:
        print(f"Could not open video source {video_source}")

    window = 'Hand Landmarks'
    cv.namedWindow(window, cv.WINDOW_NORMAL)

    gestures = []
    index_coord_list = []
    middle_coord_list = []
    middle_palm_coord_list = []
    angles = []
    last_angle = None
    prev_state = 0

    while cap.isOpened():

        sucsess, frame = cap.read()
        timestamp = time.time()*1000 # ms
        if not sucsess:
            print(f"Closed video source {video_source}")
            break

        # The Hand Landmarks model needs mp.Image to be able to run
        img = mp.Image(image_format=mp.ImageFormat.SRGB, data=frame)
        

        # Initializing model from mediapipe
        BaseOptions = mp.tasks.BaseOptions
        HandLandmarker = mp.tasks.vision.HandLandmarker
        HandLandmarkerOptions = mp.tasks.vision.HandLandmarkerOptions
        VisionRunningMode = mp.tasks.vision.RunningMode
        # We need the result_callback since using LIVE_STREAM mode
        HandLandmarkerResult = mp.tasks.vision.HandLandmarkerResult

        def set_result(result = HandLandmarkerResult, output_image = mp.Image, timestamp_ms = int):
            """
            result          (HandlandmarkerResult)  : 
            out_image       (mp.Image)              :
            timestamp_ms    (int)                   :

            When running in live-stream mode the Hand Landmarks model will not
            return the results through a HandLandmarkesResult, but rather return
            the result_callback function. To be able to use the 21 different 
            hand landmarks that is detected with the model, we need to define
            a result_callback function that saves the data. This funtion saves
            the data into the variable ret.
            """
            global ret
            ret = result

        # Create a hand landmarker instance with the live stream mode:
        options = HandLandmarkerOptions(
            base_options=BaseOptions(model_asset_path=model_path),
            running_mode=VisionRunningMode.LIVE_STREAM,
            result_callback=set_result) # Defining result_callback funtion to be ran

        # Starting the model
        with HandLandmarker.create_from_options(options) as landmarker:
            landmarker.detect_async(img, int(timestamp))

            # If the ret variable does not contain data, there is no need to run the rest of the code
            if ret and len(ret.handedness) > 0 and len(ret.hand_landmarks) > 0 and len(ret.hand_world_landmarks) > 0:

                # saving the world coordinates, which is found to be the most accurate for this system
                wrist = ret.hand_world_landmarks[0][0]
                index_tip = ret.hand_world_landmarks[0][8]
                middle_tip = ret.hand_world_landmarks[0][12]
                index_palm = ret.hand_world_landmarks[0][5]
                middle_palm = ret.hand_world_landmarks[0][9]
                
                # Finding x, y and z values of the world coordinate points
                w_x = wrist.x
                w_y = wrist.y
                w_z = wrist.z

                i_x = index_tip.x
                i_y = index_tip.y
                i_z = index_tip.z

                m_x = middle_tip.x
                m_y = middle_tip.y
                m_z = middle_tip.z

                ip_x = index_palm.x
                ip_y = index_palm.y
                ip_z = index_palm.z

                mp_x = middle_palm.x
                mp_y = middle_palm.y
                mp_z = middle_palm.z

                # Crating lists of coordinate for easier processing
                # Also easier for saving to csv file during testing and tuning
                wrist_coord = [w_x, w_y, w_z]
                index_coord = [i_x, i_y, i_z]
                middle_coord = [m_x, m_y, m_z]
                index_palm_coord = [ip_x, ip_y, ip_z]
                middle_palm_coord = [mp_x, mp_y, mp_z]

                index_coord_list.append(index_coord)
                middle_coord_list.append(middle_coord)
                middle_palm_coord_list.append(middle_palm_coord)

                # The reaon for the many variables defined is to save different data into a csv file during testing
                gesture, wrist_index_length, wrist_middle_length, index_middle_length,angle_middlepalm = find_gesture(wrist=wrist_coord, 
                                                                                                                      index=index_coord,
                                                                                                                      middle=middle_coord,
                                                                                                                      middle_palm=middle_palm_coord, 
                                                                                                                      last_angle=last_angle, 
                                                                                                                      fps=fps) 
                last_angle = angle_middlepalm
                
                angles.append(last_angle)
                gestures.append(gesture)

                state = 0

                # Defining and sending state with OSC message
                if gesture == "Standing still" or gesture == "Standing still - else":
                    state = 0
                elif gesture == "Jump":
                    state = 1
                elif gesture == "Kick":
                    state = 2
                elif gesture == "Walk":
                    state = 3
                
                if (state != prev_state):
                    client.send_message("/char_mvt/state", state)

                prev_state = state

                # Printing the detected gesture to give visual feedback to user
                print(gesture)

                # Only used during testing, possible to save world coordinates of detected region into csv file
                if filename:
                    coords = ['x','y','z']
                    data = [coords, wrist_coord, index_coord, middle_coord, index_palm_coord, middle_palm_coord]
                    length_angle = ['WI_length', 'WM_length', 'IM_length','angle_Mp']
                    data2 = [wrist_index_length, wrist_middle_length, index_middle_length,angle_middlepalm]

                    print("Saving csvfile")

                    file = open(filename, "w")
                    writer = csv.writer(file)
                    writer.writerows(data)
                    writer.writerow(length_angle)
                    writer.writerow(data2)

                    file.close()
            
            cv.imshow(window, frame) 

        
        key = cv.waitKey(1)

        # React to keyboard commands.
        if key == ord('q'):
            # Only used during testing, possible to save still image of gesture to relate to the coordinates
            if imagename:
                print("Saving last frame as image")
                path = os.getcwd()
                cv.imwrite(imagename, frame)
            print("Quitting")
            break 
      
    cap.release()
    cv.destroyAllWindows()

def find_gesture(wrist, index, middle, middle_palm, last_angle, fps):
    """
    Args:
        wrist               (list)  : list of float world coordinates of wrist
        index               (list)  : list of float world coordinates of index fingertip
        middle              (list)  : list of float world coordinates of middle fingertip
        middle_palm         (list)  : list of float world coordinates of point where the middle finger is connected to the palm
        last_angle          (float) : float value of the angle (radians) between the index and middle finger from the frame before
        fps                 (flaot) : float value of frames per seconds given by camera
    Returns a tuple of:
        gesture             (string): predicted gesture calculated from distances
        wrist_index_length  (float) : length of the vector between the wrist and the index finger
        wrist_middle_length (float) : length of the vector between the wrist and the middle finger
        index_middle_length (float) : length of the vector between the index and middle finger
        angle_middlepalm    (float) : The current angle (radians) betweeen the index and middle finger


    This function find the length of the needed vectors and calculates the angle
    between the index and middle finger. It is using the angle to determine
    which hand gesture is being captured by the webcam.
    """

    wrist_index_length = np.linalg.norm(np.array(wrist)-np.array(index))
    wrist_middle_length = np.linalg.norm(np.array(wrist)-np.array(middle))
    index_middle_length = np.linalg.norm(np.array(index)-np.array(middle))
    middlepalm_index_length = np.linalg.norm(np.array(middle_palm)-np.array(index))
    middlepalm_middle_length = np.linalg.norm(np.array(middle_palm)-np.array(middle))

    # Calculating angle in point 9 (middle_palm) because this is vissible more than index_palm
    angle_middlepalm = np.arccos((middlepalm_middle_length**2 + 
                                  middlepalm_index_length**2 - 
                                  index_middle_length**2)/(2*middlepalm_middle_length*middlepalm_index_length))

    if last_angle != None:
        angular_velocity = get_speed(angle_middlepalm, last_angle, fps)
    else:
        angular_velocity = None
    

    if angle_middlepalm < 0.30:
        gesture = "Standing still"
        if wrist_index_length < 0.13 and wrist_middle_length < 0.13:
            gesture = "Jump"
    elif angle_middlepalm > 0.72:
        gesture = "Kick"
    else:
        gesture = "Standing still"

    if (angular_velocity != None) and (angular_velocity > 1.5):
        gesture = "Walk"       

    return (gesture, wrist_index_length, wrist_middle_length, index_middle_length,angle_middlepalm)


def get_speed(angle, angle_last_frame, fps):
    """
    Args:
        angle               (float): the angle (radians) between the index and middle
        angle_last_frame    (float): the angle (radians) between the index and middle finger from the frame before
        fps                 (float): float value of frames per seconds given by camera
    Return:
        The angular velocity of the angle between the index and the middle finger.
    """

    sec = 1/fps # seconds per frame, but only 1 frame gives sec
    return abs((angle-angle_last_frame)/sec)


if __name__ == '__main__':
    run_handlandmarks(video_source=1, filename=None, imagename=None) # Running with built in web camera
