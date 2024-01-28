import os

# Get the current directory
current_directory = os.getcwd()

for item in os.listdir(current_directory):
    if os.path.isdir(item):
    
        os.chdir(os.path.join(current_directory, item))
        
        if os.path.exists('.git'):
            print(f"Pulling changes in directory: {item}")
            os.system('git pull')
        else:
            print(f"Skipping directory: {item} (Not a git repository)")
        
        os.chdir(current_directory)
