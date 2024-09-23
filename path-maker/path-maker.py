import math
import json

# Map characters to their step patterns
char_steps = {
        'A': [(0, 0, False), (1, 2, True), (2, 0, True), (0.5, 1, False)],
        'B': [(0, 0, False), (0, 2, True), (1, 1, True)],
        'C': [(2, 2, False), (0, -2, True)],
        'D': [(0, 0, False), (0, -2, True), (1, 0, True)],
        'E': [(0, 0, False), (1, 0, True), (0, -1, True), (1, -2, False)],
        'F': [(0, 0, False), (1, 0, True), (0, -1, True)],
        'G': [(2, 2, False), (0, -2, True), (1, -1, True)],
        'H': [(0, 0, False), (0, -2, True), (0.5, -1, True)],
        'I': [(0.5, 0, False), (0.5, -2, True)],
        'J': [(0.5, 0, False), (0.5, -1, True), (-0.5, -2, True)],
        'K': [(0, 0, False), (1, -1, True), (1, 1, True)],
        'L': [(0, 0, False), (0, -2, True), (1, -2, True)],
        'M': [(0, 0, False), (1, 2, True), (2, 0, True)],
        'N': [(0, 0, False), (1, 2, True), (1, 0, True)],
        'O': [(0, 0, False), (1, 1, True), (-1, -1, True)],
        'P': [(0, 0, False), (0, 2, True), (1, 1, True)],
        'Q': [(0, 0, False), (1, 1, True), (1.5, -1.5, True)],
        'R': [(0, 0, False), (0, 2, True), (1, 1, True), (1, 0, True)],
        'S': [(1, 0, False), (0, -1, True), (-1, 1, True)],
        'T': [(0.5, 2, False), (0.5, 0, True), (0, 2, False)],
        'U': [(0.5, 2, False), (0.5, 0, True), (1, 1, True)],
        'V': [(0, 2, False), (1, 0, True), (2, 2, True)],
        'W': [(0, 2, False), (1, 0, True), (2, 2, True)],
        'X': [(0, 2, False), (1, 0, True), (0, 2, True)],
        'Y': [(1, 2, False), (1, 0, True)],
        'Z': [(0, 2, False), (1, 0, True), (0, 0, False)],

        # Numbers (0-9)
        '0': [(0, 0, False), (1, -1, True), (-1, 1, True)],
        '1': [(1, 0, False), (0, 2, True)],
        '2': [(0, 0, False), (1, 0, True), (1, -1, True)],
        '3': [(0, 0, False), (1, -1, True), (0.5, 0, True)],
        '4': [(1, 0, False), (1, -2, True), (0.5, -1, True)],
        '5': [(1, 2, False), (1, 0, True), (1, -1, True)],
        '6': [(1, 2, False), (1, 0, True), (1, 2, True)],
        '7': [(1, 2, False), (0, 0, True)],
        '8': [(0, 0, False), (1, 2, True), (0, -2, True)],
        '9': [(1, 2, False), (1, 0, True)],

        # Special Symbols
        '!': [(0, 0, False), (0, -2, True), (0, -2.2, True)],
        '@': [(1, 0, False), (1, 1, True)],
        '#': [(0, 1, False), (0, -2, True), (0.5, 0.5, True), (0.5, -0.5, True)],
        '$': [(0, 1, False), (0, -1, True)],
        '%': [(0, 1, False), (0.5, 1, True)],
        '^': [(1, 0, False), (0, 2, True)],
        '&': [(0, 2, False), (1, -1, True)],
        '*': [(0, 1, False), (1, -1, True)],
        '(': [(1, 2, False), (-1, -1, True)],
        ')': [(-1, 2, False), (1, -1, True)],
        '-': [(0, -1, False), (1, -1, True)],
        '_': [(0, -1, False), (1, -1, True)],
        '+': [(0.5, 0, False), (0.5, -1, True), (0, -0.5, True)],
        '{': [(1, 2, False), (0, -1, True)],
        '}': [(-1, 2, False), (1, -1, True)],
        '[': [(1, 2, False), (0, -2, True)],
        ']': [(1, 2, False), (0, -2, True)],
        ':': [(0.5, 1, False), (0.5, -1, True)],
        ';': [(0.5, 1, False), (0.5, -1, True), (0.2, -1.2, True)],
        '<': [(1, 1, False), (0, 0, True)],
        '>': [(0, 1, False), (1, 0, True)],
        ',': [(0.5, 0, False), (0.5, -0.5, True), (0.2, -1, True)],
        '.': [(0.5, 0, False), (0.2, -1, True)],
        '?': [(0, 2, False), (1, 1, True), (0.2, -1, True)],
        '/': [(1, 2, False), (0, -1, True)]
}

# Function to calculate angles based on the arm's lengths and offsets
def calculate_angles(L1x, L1y, L2x, L2y):
    
    # Link lengths (hypotenuses)
    L1z = math.sqrt(L1x**2 + L1y**2)
    L2z = math.sqrt(L2x**2 + L2y**2)

    # Θ1: Base Rotation Angle (angle between the end effector and the horizontal axis)
    theta1 = math.atan2(L1y + L2y, L1x + L2x)  # Total displacement

    # Distance from base to end effector (resultant length)
    r = math.sqrt((L1x + L2x)**2 + (L1y + L2y)**2)

    # Θ2: Internal angle between the two links (law of cosines)
    cos_theta2 = (L1z**2 + L2z**2 - r**2) / (2 * L1z * L2z)
    
    # Adding the small offset for L2's orientation (for the slight offset you mentioned)
    theta2 = math.acos(cos_theta2) + math.atan2(L2y, L2x)

    # Return the angles in degrees for easier interpretation
    return {
        'theta1': math.degrees(theta1),
        'theta2': math.degrees(theta2)
    }

# Define the kinematics for each character
def generate_angles_for_character(char, L1x, L1y, L2x, L2y):
    steps = []  # List to store steps

    # Helper function to add a step with x, y coordinates and pen status
    def add_step(x, y, pen_down):
        step = {
            "theta1": calculate_angles(L1x + x, L1y + y, L2x, L2y)["theta1"],
            "theta2": calculate_angles(L1x + x, L1y + y, L2x, L2y)["theta2"],
            "pen": pen_down  # Whether the pen is up or down
        }
        steps.append(step)

    char = char.upper()

    # Add the steps for the specified character
    if char in char_steps:
        for step in char_steps[char]:
            add_step(step[0], step[1], step[2])

    return steps

# Function to handle a string of characters and output angles for all
def generate_angles_for_string(text, L1x, L1y, L2x, L2y):
    all_steps = {}
    
    for char in text:
        all_steps[char] = generate_angles_for_character(char, L1x, L1y, L2x, L2y)

    return all_steps

# Main function to generate angles and write to JSON
def main():
    # Example arm parameters
    L1x, L1y = 1, 0.2  # Coordinates of the first link
    L2x, L2y = 1, 0.2  # Coordinates of the second link

    # Input string: the full English keyboard characters
    input_string = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+{}[]:;<>,.?/'

    # Generate the angles for the string of characters
    angles = generate_angles_for_string(input_string, L1x, L1y, L2x, L2y)

    # Save the generated angles to a JSON file
    with open('robot_arm_angles.json', 'w') as json_file:
        json.dump(angles, json_file, indent=4)

    print(f"Angles for input '{input_string}' have been saved to 'robot_arm_angles.json'.")

# Run the main function
if __name__ == "__main__":
    main()
