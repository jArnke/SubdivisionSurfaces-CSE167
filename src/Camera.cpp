#include "Camera.h"


// HW2: You can add more helper functions if you want!


glm::mat3 rotation(const float degrees, const glm::vec3 axis) {

    const float angle = degrees * M_PI / 180.0f; // convert to radians



    // 3D rotation matrix for the given angle and axis.

    float cosTheta = glm::cos(angle);



    return cosTheta * glm::mat3(1.0f) + (1 - cosTheta) * glm::outerProduct(axis, axis) + (glm::sin(angle) * glm::mat3(0, axis[2], -axis[1], -axis[2], 0, axis[0], axis[1], -axis[0], 0));


}


void Camera::rotateRight(const float degrees) {

    glm::vec3 z = glm::normalize(eye - target);

    glm::vec3 y = glm::normalize(up - (glm::dot(up, z) * z));

    glm::vec3 x = glm::normalize(glm::cross(y, z));




    //disp = eye - target

    //disp + target = eye


    glm::vec3 newDisp = rotation(degrees, y) * (eye - target);

    eye = newDisp + target;


    // HW2: Update the class members "eye," "up"

}

void Camera::rotateUp(const float degrees) {

    glm::vec3 z = glm::normalize(eye - target);

    glm::vec3 y = glm::normalize(up - (glm::dot(up, z) * z));

    glm::vec3 x = glm::normalize(glm::cross(y, z));


    glm::vec3 newDisp = rotation(-degrees, x) * (eye - target);

    eye = newDisp + target;


    up = rotation(-degrees, x) * up;



    // HW2: Update the class members "eye," "up"

}

void Camera::computeMatrices() {

    // Note that glm matrix column majored.

    // That is, A[i] is the ith column of A,

    // and A_{ij} in math notation is A[j][i] in glm notation.



    // Update the class member "view" for the view matrix using "eye," "target," "up."

    glm::vec3 z = glm::normalize(eye - target);

    glm::vec3 y = glm::normalize(up - (glm::dot(up, z) * z));

    glm::vec3 x = glm::normalize(glm::cross(y, z));

    view = glm::inverse(glm::mat4(x[0], x[1], x[2], 0,

        y[0], y[1], y[2], 0,

        z[0], z[1], z[2], 0,

        eye[0], eye[1], eye[2], 1));




    // Update the class member "proj" for the perspective matrix using the class members "aspect," "fovy," "nearDist," "farDist."

    float fovy_rad = fovy * M_PI / 180.0f; // remember to convert degrees to radians.

    proj = glm::mat4(

        1 / (aspect * glm::tan(fovy_rad / 2)), 0.0f, 0.0f, 0.0f,

        0.0f, 1 / (glm::tan(fovy_rad / 2)), 0.0f, 0.0f,

        0.0f, 0.0f, -((farDist + nearDist) / (farDist - nearDist)), -1,

        0.0f, 0.0f, -((2 * (farDist) * (nearDist)) / (farDist - nearDist)), 0.0f);



}


void Camera::reset() {

    eye = eye_default;// position of the eye

    target = target_default;  // look at target

    up = up_default;      // up vector

    fovy = fovy_default;  // field of view in degrees

    aspect = aspect_default; // aspect ratio

    nearDist = near_default; // nearDist clipping distance

    farDist = far_default; // farDist clipping distance

}
