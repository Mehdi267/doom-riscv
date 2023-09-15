import os
import sys

if len(sys.argv) != 2:
    print("Usage: python generate_c_files.py source_directory")
    sys.exit(1)

# Command-line arguments
source_directory = sys.argv[1]
output_file = "copy_files.c"

# Ensure the source directory exists
if not os.path.isdir(source_directory):
    print("Source directory does not exist.")
    sys.exit(1)

# Function to generate C code for a file
def generate_c_code(file_path, filename):
    with open(file_path, 'rb') as source_file:
        source_data = source_file.read()

    filenameData = f'fileData{filename.split(".")[0]}'
    with open(output_file, 'a') as c_file:
        c_file.write(f'    const unsigned char {filenameData}[] = {{\n')
        for byte in source_data[:-1]:
            c_file.write(f'0x{byte:02X},')
        c_file.write(f'0x{source_data[-1]:02X}\n')
        c_file.write('    };\n\n')
        c_file.write(f'// Function for {filename}\n')
        c_file.write(f'void copy_{filename.split(".")[0]}() {{\n')
        c_file.write(f'    int outFile = open("{filename}", O_CREAT | O_WRONLY, 0);\n')
        c_file.write('    if (outFile == -1) {\n')
        c_file.write('        perror("Error opening output file\\n");\n')
        c_file.write('        return;\n')
        c_file.write('    }\n\n')
        
        c_file.write(f'    size_t dataSize = sizeof({filenameData});\n')
        c_file.write(f'    size_t elementsWritten = write(outFile, {filenameData}, dataSize);\n')
        c_file.write('    if (elementsWritten != dataSize) {\n')
        c_file.write('        perror("Error writing to output file\\n");\n')
        c_file.write('        close(outFile);\n')
        c_file.write('        return;\n')
        c_file.write('    }\n\n')
        c_file.write('    close(outFile);\n')
        c_file.write(f'   printf("Data from {filename} written successfully \\n");')
        c_file.write('}\n\n')

# Main function that calls the generated copy functions
def main():
    # Remove the existing output file if it exists
    if os.path.exists(output_file):
      os.remove(output_file)
    with open(output_file, 'w') as c_file:
      c_file.write(f'#include <stdio.h> \n')
      c_file.write(f'#include <unistd.h> \n')

    # Generate C functions for files in the source directory
    for filename in os.listdir(source_directory):
      source_file_path = os.path.join(source_directory, filename)
      generate_c_code(source_file_path, filename)
    print("Starting data copy...")
    with open(output_file, 'a') as c_file:
      # Call the generated copy functions for each input file
      c_file.write(f'// main func\n')
      c_file.write(f'void main(){{ \n')
      for filename in os.listdir(source_directory):
          c_file.write(f'copy_{filename.split(".")[0]}();\n')
      c_file.write('}\n')
    print("Data copy completed.")

if __name__ == "__main__":
    main()
