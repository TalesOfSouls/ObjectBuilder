#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <limits.h>
#endif

#include "../GameEngine/log/Debug.h"
#include "../GameEngine/log/Debug.cpp"

#include "../GameEngine/object/Mesh.h"
#include "../GameEngine/object/Vertex.h"
#include "../GameEngine/object/Material.h"
#include "../GameEngine/object/Hitbox.h"
#include "../GameEngine/object/Animation.h"
#include "../GameEngine/utils/StringUtils.h"

Mesh* meshes;
Material* materials;
Animation* animations;
Hitbox* hitboxes;
int mesh_index = 0;
int material_index = 0;
int animation_index = 0;
int hitbox_index = 0;

#if _WIN32
void iter_directories_recursive(RingMemory* ring, const char *dir_path) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char path[MAX_PATH];
    char searchPath[MAX_PATH];

    snprintf(searchPath, sizeof(searchPath), "%s\\*", dir_path);

    hFind = FindFirstFileA(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "FindFirstFile failed (%d)\n", GetLastError());
        return;
    }

    do {
        if (findFileData.cFileName[0] == '.') {
            continue;
        }

        snprintf(path, sizeof(path), "%s\\%s", dir_path, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            iter_directories_recursive(ring, path);
        } else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
            const char *ext = strrchr(findFileData.cFileName, '.');

            if (ext && strcmp(ext, ".objtxt") == 0) {
                char abs_path[MAX_PATH];
                if (GetFullPathNameA(path, sizeof(abs_path), abs_path, NULL)) {
                    printf("Found .objtxt file: %s\n", abs_path);

                    meshes[mesh_index].data = (byte *) calloc(10, MEGABYTE);

                    FileBody file;
                    file_read(abs_path, &file, ring);
                    mesh_from_file_txt(meshes + mesh_index, file.content, ring);

                    char new_path[MAX_PATH];
                    str_replace(abs_path, ".objtxt", ".objbin", new_path);
                    mesh_to_file(ring, new_path, meshes + mesh_index, VERTEX_TYPE_ALL, 8);

                    free(meshes[mesh_index].data);

                    ++mesh_index;
                }
            } else if (ext && strcmp(ext, ".mtl") == 0) {
                char abs_path[MAX_PATH];
                if (GetFullPathNameA(path, sizeof(abs_path), abs_path, NULL)) {
                    printf("Found .mtl file: %s\n", abs_path);
                    material_from_file_txt(ring, abs_path, &materials[material_index]);

                    ++material_index;
                }
            } else if (ext && strcmp(ext, ".ani") == 0) {
                char abs_path[MAX_PATH];
                if (GetFullPathNameA(path, sizeof(abs_path), abs_path, NULL)) {
                    printf("Found .ani file: %s\n", abs_path);
                    animation_from_file_txt(ring, abs_path, &animations[animation_index]);

                    ++animation_index;
                }
            } else if (ext && strcmp(ext, ".hit") == 0) {
                char abs_path[MAX_PATH];
                if (GetFullPathNameA(path, sizeof(abs_path), abs_path, NULL)) {
                    printf("Found .hit file: %s\n", abs_path);
                    hitbox_from_file_txt(ring, abs_path, &hitboxes[hitbox_index]);

                    ++hitbox_index;
                }
            }
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
}
#else
void iter_directories_recursive(RingMemory* ring, const char *dir_path) {
    struct dirent *entry;
    DIR *dir = opendir(dir_path);

    if (!dir) {
        fprintf(stderr, "Could not open directory: %s\n", dir_path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        struct stat statbuf;

        if (entry->d_name == '.') {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
        if (stat(path, &statbuf) != 0) {
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            iter_directories_recursive(ring, path);
        } else if (S_ISREG(statbuf.st_mode)) {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".objtxt") == 0) {
                char abs_path[PATH_MAX];
                if (realpath(path, abs_path)) {
                    printf("Found .objtxt file: %s\n", abs_path);

                    meshes[mesh_index].data = (Vertex3D *) calloc(100000, sizeof(Vertex3D));
                    meshes[mesh_index].materials = (uint32 *) malloc(sizeof(uint32) * 100);
                    meshes[mesh_index].animations = (uint32 *) malloc(sizeof(uint32) * 300);
                    meshes[mesh_index].hitboxes = (uint32 *) malloc(sizeof(uint32) * 100);
                    meshes[mesh_index].audios = (uint32 *) malloc(sizeof(uint32) * 100);

                    FileBody file;
                    file_read(abs_path, &file, ring);
                    mesh_from_file_txt(meshes + mesh_index, file.content);

                    char new_path[MAX_PATH];
                    str_replace(abs_path, ".objtxt", ".objbin", new_path);
                    mesh_to_file(ring, new_path, meshes + mesh_index, VERTEX_TYPE_ALL, 8);

                    free(meshes[mesh_index].vertices);
                    free(meshes[mesh_index].materials);
                    free(meshes[mesh_index].animations);
                    free(meshes[mesh_index].hitboxes);
                    free(meshes[mesh_index].audios);

                    ++mesh_index;
                }
            } else if (ext && strcmp(ext, ".mtl") == 0) {
                char abs_path[MAX_PATH];
                if (GetFullPathNameA(path, sizeof(abs_path), abs_path, NULL)) {
                    printf("Found .mtl file: %s\n", abs_path);
                    material_from_file_txt(ring, abs_path, &materials[material_index]);

                    ++material_index;
                }
            } else if (ext && strcmp(ext, ".ani") == 0) {
                char abs_path[MAX_PATH];
                if (GetFullPathNameA(path, sizeof(abs_path), abs_path, NULL)) {
                    printf("Found .ani file: %s\n", abs_path);
                    animation_from_file_txt(ring, abs_path, &animations[animation_index]);

                    ++animation_index;
                }
            } else if (ext && strcmp(ext, ".hit") == 0) {
                char abs_path[MAX_PATH];
                if (GetFullPathNameA(path, sizeof(abs_path), abs_path, NULL)) {
                    printf("Found .hit file: %s\n", abs_path);
                    hitbox_from_file_txt(ring, abs_path, &hitboxes[hitbox_index]);

                    ++hitbox_index;
                }
            }
        }
    }

    closedir(dir);
}
#endif

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    RingMemory memory_volatile = {};
    memory_volatile.memory = (byte *) malloc(sizeof(byte) * GIGABYTE * 1);
    memory_volatile.size = sizeof(byte) * GIGABYTE * 1;

    int mesh_count = 1000;
    meshes = (Mesh *) malloc(sizeof(Mesh) * mesh_count);

    materials = (Material *) malloc(sizeof(Material) * 1000);
    animations = (Animation *) malloc(sizeof(Animation) * 1000);
    hitboxes = (Hitbox *) malloc(sizeof(Hitbox) * 1000);

    iter_directories_recursive(&memory_volatile, argv[1]);

    printf("Meshes %d\n", mesh_index);
}
