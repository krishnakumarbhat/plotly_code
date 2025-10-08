import os, sys, getpass, datetime


version = '1.0'
aptiv = f'''
\t************************************************************************
\t*           ___       ______  _________ _________ __          __       *
\t*          / _ \     |  __  | \__   __/ \__   __/ \ \        / /       *
\t*         / / \ \    | |__| |    | |       | |     \ \      / /        *
\t*        / /___\ \   |  ____|    | |       | |      \ \    / /         *
\t*       / ______\ \  | |         | |     __| |__     \ \__/ /          *
\t*      /_/       \_\ |_|         |_|    /_______\     \____/           *
\t*                                                                      *
\t************************************************************************
\t*                                                                      *
\t*               ReSET:Resim Singularity Execution Tool                 *
\t*                                                                      *
\t************************************************************************
\t************************************************************************
\t* version: {version}                   Help : mandeep.singh1@aptiv.com       *
\t************************************************************************
****************************************************************************************'''

base_out = "#SBATCH -o"
base_err = "#SBATCH -e"

base_child = '''#!/bin/bash
#SBATCH --job-name=html_Smoke           
#SBATCH --nodes=1
#SBATCH --ntasks=1               
#SBATCH --cpus-per-task=6  
#SBATCH --mem=4G
#SBATCH --time=00:30:00
#SBATCH -o /mnt/usmidet/projects/STLA-THUNDER/2-Sim/USER_DATA/d1cse7/work/RESIM/playground/jobout/slurmout_%A_%a.log
#SBATCH -e /mnt/usmidet/projects/STLA-THUNDER/2-Sim/USER_DATA/d1cse7/work/RESIM/playground/jobout/slurmout_%A_%a.log


ARRAY_START="$1"
SIMG_PATH="$2"
RESULT_OUT="$3/output"
RESULT_JOB="$3/jobout"
INPUT_LIST=$4
HTML_CONFIG=$5

echo $1
echo $2
echo $3
echo $4
echo $5

# Start application
echo "______________ Start of the application ______________"
LINE=$(( $SLURM_ARRAY_TASK_ID + $ARRAY_START ))
echo "LINE_NUMBER= $LINE"
INPUTFILE=$(sed -n ${LINE}p "${INPUT_LIST}")
echo ${INPUTFILE}


module load singularity/3.8.0
SIMG_START=$SECONDS
echo "HTML starts at  $SECONDS"
echo "$SIMG_PATH /RUN_HTML.sh $HTML_CONFIG $INPUTFILE $RESULT_OUT/$SLURM_ARRAY_TASK_ID"
singularity exec $SIMG_PATH /RUN_HTML.sh $HTML_CONFIG $INPUTFILE "$RESULT_OUT/$SLURM_ARRAY_TASK_ID"
echo "HTML ends at  $SECONDS"
echo "HTML took $(( $SECONDS - $SIMG_START )) to complete"
echo "TOTAL TIME TAKEN to complete entire job is $(( $SECONDS - $SIMG_START )) "

'''


#**********************************************************
#          function to validate inputs provided
#**********************************************************
def validate_inputs():
    print('[INFO] : validating Inputs', end='\r')
    #print(len(sys.argv),' : ',sys.argv)
    def check_input_files(files):
        for file in files:
            if not os.path.isfile(file):
                print('[INFO] : validated  Inputs')
                print(f"[ERROR] : invalid file - {file} does not exist")
                exit()
        return True
    
    if len(sys.argv) <= 2:
        print('[INFO] : validated  Inputs')
        print("[ERROR] : required input - html_smoketest.sh <html_singularity> <generation (gen6/gen7)>")
        exit()
    if sys.argv[2].lower() == "gen6" or sys.argv[2].lower() == "gen7":
        smoketest_txt = f"/mnt/usmidet/projects/STLA-THUNDER/7-Tools/Resim/InternalUpload/HTML_DOCKER/SmokeTest/config/html_smoketest_jsonpath_{sys.argv[2].lower()}.txt"
        config=f"/mnt/usmidet/projects/STLA-THUNDER/7-Tools/Resim/InternalUpload/HTML_DOCKER/SmokeTest/config/HTMLConfig_{sys.argv[2].lower()}.xml"
    else:
        print('[INFO] : validated  Inputs')
        print(f"[ERROR] : Unsupported generation - Accepted generation in version {version} is <gen6> || <gen7>")
        exit()

    if ".simg" not in sys.argv[1]:
        print('[INFO] : validated  Inputs')
        print(f"[ERROR] : passed image is not a singularity image")
        exit()
    else:
        validated=check_input_files([sys.argv[1],smoketest_txt,config])
    
    if validated:
        print('[INFO] : validated  Inputs')
        script = "/mnt/usmidet/projects/STLA-THUNDER/7-Tools/ReSimAutoMng/Shell/rR_html_smoketest_main.sh"
        return script, sys.argv[1], smoketest_txt, config
    
    

#**********************************************************
#          function to create slurm script to get resource and run job
#**********************************************************
def create_QualityChecker_script(inputfile):
    print('[INFO] : configuring tool Script', end='\r')
    if "projects" in os.getcwd() :
        server = "projects/"
    elif "PROJECTS" in os.getcwd() :
        server = "PROJECTS/"
    else:
        server="NOT defined"
    path_to_pick = os.getcwd().split(server)
    pre_path= path_to_pick[0]+server
    project= path_to_pick[1].split('/')[0]
    user = getpass.getuser()
    datentime = str(datetime.datetime.now())
    date = datentime.split(' ')[0]
    time = datentime.split(' ')[1].replace(':','-').replace('.','-')
    path = f"{pre_path}{project}/2-Sim/USER_DATA/{user}/TOOL-RESULTS/{date}/{time}"
    jobout_path = f"{path}/jobout"
    output_path = f"{path}/output"
    new_out = f"#SBATCH -o {jobout_path}/%a/%A_%a.out"
    new_err = f"#SBATCH -e {jobout_path}/%a/%A_%a.out"
    os.makedirs(jobout_path)
    os.makedirs(output_path)
    
    base_child_data = base_child.split('\n')
    file = open(f"{jobout_path}/.html.sh",'w')
    for line in base_child_data:
        if base_out in line:
            file.write(f"{new_out}\n")
        elif base_err in line:
            file.write(f"{new_err}\n")
        else:
            file.write(f'{line}\n')
    file.close()
    file=open(inputfile, 'r')
    sessions=file.readlines()
    file.close()
    
    for i in range(1,len(sessions)+1):
        os.makedirs(f'{jobout_path}/{i}')
        os.makedirs(f'{output_path}/{i}')

    print(f'[INFO] : configured  tool Script\n[OUTPUT_PATH] : {path}')
    return path


#**********************************************************
#          main function to trigger Quality Tool
#**********************************************************
if __name__ == '__main__':
    inputs_parameters = validate_inputs()
    print(aptiv)
    job_path = create_QualityChecker_script(inputs_parameters[2])
    os.system(f" {inputs_parameters[0]} {inputs_parameters[1]} {job_path} {inputs_parameters[2]} {inputs_parameters[3]} ")
    
