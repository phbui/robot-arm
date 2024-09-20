import math
import json

# Function to calculate angles based on the arm's lengths and offsets
def calculate_angles(L1x, L1y, L2x, L2y, O1, O2):
    # Link lengths
    L1z = math.sqrt(L1x**2 + L1y**2)
    L2z = math.sqrt(L2x**2 + L2y**2)

    # Θ1: Base Rotation Angle
    theta1 = math.atan2(L2z, L1z) + O1

    # r: Distance between the two links
    r = math.sqrt(L1z**2 + L2z**2)

    # Θ2: Arm Tilt Angle
    cos_theta2 = (L1z**2 + L2z**2 - r**2) / (2 * L1z * L2z)
    theta2 = math.acos(cos_theta2) + O2

    # Return the angles in degrees for easier interpretation
    return {
        'theta1': math.degrees(theta1),
        'theta2': math.degrees(theta2)
    }

# Define the kinematics for each character
def generate_angles_for_character(char, L1x, L1y, L2x, L2y, O1, O2):
    angle_steps = {}

    char = char.upper()

    # Uppercase Letters (A-Z)
    if char == 'A':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_up_right'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_down_right'] = calculate_angles(L1x + 2, L1y, L2x, L2y, O1, O2)
        angle_steps['move_across'] = calculate_angles(L1x + 0.5, L1y + 1, L2x, L2y, O1, O2)
    
    elif char == 'B':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_up'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)

    elif char == 'C':
        angle_steps['move_to_start'] = calculate_angles(L1x + 2, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_curve_down_left'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)

    elif char == 'D':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)
        angle_steps['curve_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)

    elif char == 'E':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_across_top'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down_middle'] = calculate_angles(L1x, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['move_across_bottom'] = calculate_angles(L1x + 1, L1y - 2, L2x, L2y, O1, O2)

    elif char == 'F':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_across_top'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down_middle'] = calculate_angles(L1x, L1y - 1, L2x, L2y, O1, O2)

    elif char == 'G':
        angle_steps['move_to_start'] = calculate_angles(L1x + 2, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_curve_down_left'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)
        angle_steps['move_right'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == 'H':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)
        angle_steps['move_across_middle'] = calculate_angles(L1x + 0.5, L1y - 1, L2x, L2y, O1, O2)

    elif char == 'I':
        angle_steps['move_to_start'] = calculate_angles(L1x + 0.5, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 0.5, L1y - 2, L2x, L2y, O1, O2)

    elif char == 'J':
        angle_steps['move_to_start'] = calculate_angles(L1x + 0.5, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 0.5, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['move_curve_left'] = calculate_angles(L1x - 0.5, L1y - 2, L2x, L2y, O1, O2)

    elif char == 'K':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_left'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_up_left'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)

    elif char == 'L':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)
        angle_steps['move_right'] = calculate_angles(L1x + 1, L1y - 2, L2x, L2y, O1, O2)

    elif char == 'M':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_up_right'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_right'] = calculate_angles(L1x + 2, L1y, L2x, L2y, O1, O2)

    elif char == 'N':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_up_right'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)

    elif char == 'O':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_curve_right'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_curve_left'] = calculate_angles(L1x - 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == 'P':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_up'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_curve_right'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)

    elif char == 'Q':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_curve_right'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down'] = calculate_angles(L1x + 1.5, L1y - 1.5, L2x, L2y, O1, O2)

    elif char == 'R':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_up'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_curve_right'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)

    elif char == 'S':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_curve_down_left'] = calculate_angles(L1x, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['move_curve_up_left'] = calculate_angles(L1x - 1, L1y + 1, L2x, L2y, O1, O2)

    elif char == 'T':
        angle_steps['move_to_start'] = calculate_angles(L1x + 0.5, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 0.5, L1y, L2x, L2y, O1, O2)
        angle_steps['move_across_top'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)

    elif char == 'U':
        angle_steps['move_to_start'] = calculate_angles(L1x + 0.5, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 0.5, L1y, L2x, L2y, O1, O2)
        angle_steps['move_curve_right'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)

    elif char == 'V':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_up_right'] = calculate_angles(L1x + 2, L1y + 2, L2x, L2y, O1, O2)

    elif char == 'W':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_up_right'] = calculate_angles(L1x + 2, L1y + 2, L2x, L2y, O1, O2)

    elif char == 'X':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_up_left'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)

    elif char == 'Y':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)

    elif char == 'Z':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_across_bottom'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)

    # Numbers (0-9)
    elif char == '0':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['curve_down_right'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['curve_up_left'] = calculate_angles(L1x - 1, L1y + 1, L2x, L2y, O1, O2)

    elif char == '1':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_up'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)

    elif char == '2':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down_right'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == '3':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['curve_down_right'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['move_across'] = calculate_angles(L1x + 0.5, L1y, L2x, L2y, O1, O2)

    elif char == '4':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 1, L1y - 2, L2x, L2y, O1, O2)
        angle_steps['move_across_middle'] = calculate_angles(L1x + 0.5, L1y - 1, L2x, L2y, O1, O2)

    elif char == '5':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['curve_right'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == '6':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve_down_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_up'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)

    elif char == '7':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_left'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)

    elif char == '8':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['curve_up_right'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve_down_left'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)

    elif char == '9':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve_down_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)


    # Special Symbols
    elif char == '!':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)
        angle_steps['move_dot'] = calculate_angles(L1x, L1y - 2.2, L2x, L2y, O1, O2)

    elif char == '@':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['curve_inward'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)

    elif char == '#':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)
        angle_steps['move_horizontal_1'] = calculate_angles(L1x + 0.5, L1y + 0.5, L2x, L2y, O1, O2)
        angle_steps['move_horizontal_2'] = calculate_angles(L1x + 0.5, L1y - 0.5, L2x, L2y, O1, O2)

    elif char == '$':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['curve_down'] = calculate_angles(L1x, L1y - 1, L2x, L2y, O1, O2)

    # Define the rest of the special symbols
    elif char == '%':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_across'] = calculate_angles(L1x + 0.5, L1y + 1, L2x, L2y, O1, O2)

    elif char == '^':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_up'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)

    elif char == '&':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve_down_right'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)
    
    elif char == '*':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_cross'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == '%':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_across'] = calculate_angles(L1x + 0.5, L1y + 1, L2x, L2y, O1, O2)

    elif char == '^':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_up'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)

    elif char == '&':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve_down_right'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == '*':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_cross'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == '(':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve_down_left'] = calculate_angles(L1x - 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == ')':
        angle_steps['move_to_start'] = calculate_angles(L1x - 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve_down_right'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == '_':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['move_across'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)
        # Special Symbols and Empty Characters (Example)
    elif char == '-':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['move_across'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == '=':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y - 0.5, L2x, L2y, O1, O2)
        angle_steps['move_across_top'] = calculate_angles(L1x + 1, L1y - 0.5, L2x, L2y, O1, O2)
        angle_steps['move_across_bottom'] = calculate_angles(L1x + 1, L1y - 1.5, L2x, L2y, O1, O2)

    elif char == '+':
        angle_steps['move_to_start'] = calculate_angles(L1x + 0.5, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 0.5, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['move_across'] = calculate_angles(L1x, L1y - 0.5, L2x, L2y, O1, O2)

    elif char == '{':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve_down_left'] = calculate_angles(L1x, L1y - 1, L2x, L2y, O1, O2)

    elif char == '}':
        angle_steps['move_to_start'] = calculate_angles(L1x - 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['curve_down_right'] = calculate_angles(L1x + 1, L1y - 1, L2x, L2y, O1, O2)

    elif char == '[':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)
        angle_steps['move_across_bottom'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)

    elif char == ']':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x, L1y - 2, L2x, L2y, O1, O2)
        angle_steps['move_across_bottom'] = calculate_angles(L1x + 1, L1y - 2, L2x, L2y, O1, O2)

    elif char == ':':
        angle_steps['move_to_start'] = calculate_angles(L1x + 0.5, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 0.5, L1y - 1, L2x, L2y, O1, O2)

    elif char == ';':
        angle_steps['move_to_start'] = calculate_angles(L1x + 0.5, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 0.5, L1y - 1, L2x, L2y, O1, O2)
        angle_steps['move_curve_dot'] = calculate_angles(L1x + 0.2, L1y - 1.2, L2x, L2y, O1, O2)

    elif char == '<':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_left'] = calculate_angles(L1x, L1y, L2x, L2y, O1, O2)

    elif char == '>':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_right'] = calculate_angles(L1x + 1, L1y, L2x, L2y, O1, O2)

    elif char == ',':
        angle_steps['move_to_start'] = calculate_angles(L1x + 0.5, L1y, L2x, L2y, O1, O2)
        angle_steps['move_down'] = calculate_angles(L1x + 0.5, L1y - 0.5, L2x, L2y, O1, O2)
        angle_steps['move_curve_dot'] = calculate_angles(L1x + 0.2, L1y - 1, L2x, L2y, O1, O2)

    elif char == '.':
        angle_steps['move_to_start'] = calculate_angles(L1x + 0.5, L1y, L2x, L2y, O1, O2)
        angle_steps['move_dot'] = calculate_angles(L1x + 0.2, L1y - 1, L2x, L2y, O1, O2)

    elif char == '?':
        angle_steps['move_to_start'] = calculate_angles(L1x, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_curve_right'] = calculate_angles(L1x + 1, L1y + 1, L2x, L2y, O1, O2)
        angle_steps['move_dot'] = calculate_angles(L1x + 0.2, L1y - 1, L2x, L2y, O1, O2)

    elif char == '/':
        angle_steps['move_to_start'] = calculate_angles(L1x + 1, L1y + 2, L2x, L2y, O1, O2)
        angle_steps['move_diagonal_down_left'] = calculate_angles(L1x, L1y - 1, L2x, L2y, O1, O2)


    return angle_steps

# Function to handle a string of characters and output angles for all
def generate_angles_for_string(text, L1x, L1y, L2x, L2y, O1, O2):
    all_steps = {}
    
    for char in text:
        all_steps[char] = generate_angles_for_character(char, L1x, L1y, L2x, L2y, O1, O2)

    return all_steps

# Main function to generate angles and write to JSON
def main():
    # Example arm parameters
    L1x, L1y = 5, 5  # Coordinates of the first link
    L2x, L2y = 3, 3  # Coordinates of the second link
    O1 = 0.1         # Offset 1 (slight offset for Θ1)
    O2 = 0.05        # Offset 2 (slight offset for Θ2)

    # Input string: the full English keyboard characters
    input_string = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+{}[]:;<>,.?/'

    # Generate the angles for the string of characters
    angles = generate_angles_for_string(input_string, L1x, L1y, L2x, L2y, O1, O2)

    # Save the generated angles to a JSON file
    with open('robot_arm_angles.json', 'w') as json_file:
        json.dump(angles, json_file, indent=4)

    print(f"Angles for input '{input_string}' have been saved to 'robot_arm_angles.json'.")

# Run the main function
if __name__ == "__main__":
    main()
