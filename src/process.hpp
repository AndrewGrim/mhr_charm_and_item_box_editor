#ifndef PROCESS_HPP
    #define PROCESS_HPP

    #include <Agro/common/number_types.h>

    #define DATA_MANAGER_ADDRESS 0x14C0578C0
    #define MONSTER_HUNTER_RISE_EXE "MonsterHunterRise.exe"

    #ifdef __WIN32__
        #include <Windows.h>
        #include <tlhelp32.h>

        struct Process {
            HANDLE id = NULL;

            Process() {}
            ~Process() {}

            Process open(const char *executable_name) {
                PROCESSENTRY32 entry;
                ZeroMemory(&entry, sizeof(PROCESSENTRY32));
                entry.dwSize = sizeof(PROCESSENTRY32);

                HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
                if (Process32First(snapshot, &entry)) {
                    while (Process32Next(snapshot, &entry)) {
                        if (strcmp(executable_name, entry.szExeFile) == 0) {
                            id = OpenProcess(PROCESS_ALL_ACCESS, false, entry.th32ProcessID);
                            if (!id || id == INVALID_HANDLE_VALUE) {
                                assert(false && "Failed to open executable_name");
                            }
                            return *this;
                        }
                    }
                }
                assert(false && "Could not find executable_name in the currently running processes!");
                return *this;
            }

            bool read(usize read_from, void *write_to, usize number_of_bytes) {
                return ReadProcessMemory(id, (void*)read_from, write_to, number_of_bytes, NULL);
            }

            bool write(usize write_to, void *read_from, usize number_of_bytes) {
                return WriteProcessMemory(id, (void*)write_to, read_from, number_of_bytes, NULL);
            }
        };
    #else
        #include <dirent.h>
        #include <sys/uio.h>

        #define PROC_DIRECTORY "/proc/"

        i32 IsNumeric(const char *string) {
            for (; *string; string++)
                if (*string < '0' || *string > '9')
                    return 0;
            return 1;
        }

        struct Process {
            pid_t id = -1;

            Process() {}
            ~Process() {}

            Process& open(const char *executable_name) {
                char chrarry_CommandLinePath[800];
                char chrarry_NameOfProcess[500];
                char *process_name = NULL;
                struct dirent *de_DirEntity = NULL;
                DIR *dir_proc = NULL;

                dir_proc = opendir(PROC_DIRECTORY);
                if (dir_proc == NULL) {
                    assert(false && "Couldn't open the " PROC_DIRECTORY " directory");
                }

                while ((de_DirEntity = readdir(dir_proc))) {
                    // Skip non numeric entries
                    if (de_DirEntity->d_type == DT_DIR) {
                        if (IsNumeric(de_DirEntity->d_name)) {
                            strcpy(chrarry_CommandLinePath, PROC_DIRECTORY);
                            strcat(chrarry_CommandLinePath, de_DirEntity->d_name);
                            strcat(chrarry_CommandLinePath, "/cmdline");
                            FILE* fd_CmdLineFile = fopen (chrarry_CommandLinePath, "rt");  // open the file for reading text
                            if (fd_CmdLineFile) {
                                fscanf(fd_CmdLineFile, "%s", chrarry_NameOfProcess); // read from /proc/<NR>/cmdline
                                fclose(fd_CmdLineFile);  // close the file prior to exiting the routine

                                if (strrchr(chrarry_NameOfProcess, '\\')) {
                                    process_name = strrchr(chrarry_NameOfProcess, '\\') + 1;
                                } else {
                                    process_name = chrarry_NameOfProcess;
                                }

                                if (strcmp(process_name, executable_name) == 0) {
                                    id = atoi(de_DirEntity->d_name);
                                    closedir(dir_proc);
                                    return *this;
                                }
                            }
                        }
                    }
                }
                closedir(dir_proc);
                assert(false && "Could not find executable_name in the currently running processes!");
                return *this;
            }

            bool read(usize read_from, void *write_to, usize number_of_bytes) {
                iovec local = { write_to, number_of_bytes };
                iovec remote = { (void*)read_from, number_of_bytes };
                return process_vm_readv(id, &local, 1, &remote, 1, 0) != -1;
            }

            bool write(usize write_to, void *read_from, usize number_of_bytes) {
                iovec local = { read_from, number_of_bytes };
                iovec remote = { (void*)write_to, number_of_bytes };
                return process_vm_writev(id, &local, 1, &remote, 1, 0) != -1;
            }
        };
    #endif
#endif

