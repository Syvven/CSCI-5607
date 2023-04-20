# Noah Hendrickson -- Assignment 2b

## Controls

- WASD
    - move in corresponding direction 
    - moves w.r.t. the view direction
        - i.e. looking down and holding w will move camera down in y
- Up/Down
    - change the pitch of the camera
- Left/Right
    - change the yaw of the camera
    - both of the previous are controlled by euler angles
    - I had wanted to try quaternions but couldn't get them working
- Space/LeftControl
    - space ascends in global y
    - left control descends in global y
    - these are far more intuitive controls than [] so yeah
    - also personally I would have had it ascend/descend corresponding to the camera's up direction but whatever
- Shift
    - speeds up the movement when held
    - a sprint button perhaps
- Window Resize
    - on window resize, the contents are changed as well
    - if resized to 1:1 aspect ratio but smaller/bigger, the contents will get smaller/bigger
    - if resized to x:y aspect ratio where y =/= x, the smaller of the two will be preserved and the larger axis will have more shown
        - i.e. if the width is bigger than the height, you will be able to see more to the left and right but the height will stay the same

## Extras

I did both extra credit as shown above.

Also, its not necessary for things to work but there are some funny kiwis rotating! <br>
I wanted to make them into BOIDS that would roam the main hall of the church, <br>
but I ended up dealing with an issue for like the whole week and didn't get to it. <br>
The skeleton for updating each individual transformation matrix is there though! <br>
Additionally, for simplicity sake, the sim runs just find without the kiwi obj file present. <br>
In order for this to work I did modify trimesh.hpp (because of a bug with quad reading) <br>
 so I will be submitting that as well. <br>
 I will also submit the kiwi obj file in a folder called "biwer". <br>
 In order for this to be loaded properly, the biwer folder must be on the same level <br> as the folders for the church. <br>
i.e. the directory structure should look like:

- data
    - biwer
        - kiwi1.obj
    - sibenik
        - ...
    - sponza
        - ...
- ...
