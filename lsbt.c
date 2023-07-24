#include <unistd.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <regex.h>   
#include <sys/ioctl.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include <limits.h>
#include <stdio.h>

void print_file(char* dir_name, struct stat sb);
void print_mode(mode_t m, char* mode);
int regex_test(char * string, char * reg);
int hidden_filter(const struct dirent *dir);
int get_term_width(void);
int intlen(int n);
void printspace(int n);
int width_lines(struct dirent **dir_list, int nb_c, int nb_l, int nb_file);
int width_column(struct dirent **dir_list, int nb_c, int nb_l, int num_c, int nb_file);
void print_line(struct dirent **dir_list, int dim_c[], int nb_c, int nb_l, int num_l, int nb_file);

size_t mbStrlen( const char * str );

char units[] = {' ', 'K', 'M', 'G', 'T'};

char * folder_name = ".";

int op_a = 0, op_l = 0, op_h = 0;

void main(int argc, char** argv){
    setlocale( LC_ALL, "en_US.utf8" ); 
    
    DIR *d;
    struct dirent *dir;
    struct dirent **dir_list;
    int nb_file, nb_cur;
    FILE * f;
    int nbPattern = 0;
    int i;

    char mode[11] = {};

    struct stat sb;
    struct passwd *passwd;
    struct group *group;

    int term_width = get_term_width();
    //printf("%d\n", get_term_width());

    switch(argc)
    { 
    case 1:
        nb_file = scandir(folder_name, &dir_list, hidden_filter, alphasort);
        break;
    default:
        if (argv[1][0] == '-'){
            if (argc>2) folder_name = argv[2];

			char *p = (char*)(argv[1] + 1);
			while(*p){
				if(*p == 'a'){
                    op_a = 1;
                }
				else if(*p == 'l'){
                    op_l = 1;
                }
                else if(*p == 'h'){
                    op_h = 1;
                }
				else{
					perror("Option not available");
					exit(EXIT_FAILURE);
				}
				p++;
			}
			if(op_a) nb_file = scandir(folder_name, &dir_list, NULL, alphasort);
            else nb_file = scandir(folder_name, &dir_list, hidden_filter, alphasort);
            
		} else {
            folder_name = argv[1];
            nb_file = scandir(folder_name, &dir_list, hidden_filter, alphasort);
        }

        break;
    }

    //nb_file = scandir(".", &dir_list, hidden_filter, alphasort);
    if (nb_file == -1) {
        perror("lsbt");
        exit(EXIT_FAILURE);
    }
    nb_cur = nb_file;

    /*
        Get listed format dimensions for formatting
    */
    int dim[4] = {0, 0, 0, 0};
    while (nb_cur--) {
        dir = dir_list[nb_file-nb_cur-1];
        
        char * file_path =malloc((strlen(folder_name)+strlen(dir->d_name)+2)*sizeof(char));
        sprintf(file_path, "%s/%s", folder_name, dir->d_name);
        lstat(file_path, &sb);

        if (intlen(sb.st_nlink)>dim[0]) dim[0] = intlen(sb.st_nlink);

        passwd = getpwuid(sb.st_uid);  
        if (mbStrlen(passwd->pw_name)>dim[1]) dim[1] = mbStrlen(passwd->pw_name);

        group = getgrgid(sb.st_gid);
        if (mbStrlen(group->gr_name)>dim[2]) dim[2] = mbStrlen(group->gr_name);

        if (intlen(sb.st_size)>dim[3]) dim[3] = intlen(sb.st_size);
    }
    //printf("%d %d %d %d \n", dim[0], dim[1], dim[2], dim[3]);

    nb_cur = nb_file;
    if(op_l){
        while (nb_cur--) {
            dir = dir_list[nb_file-nb_cur-1];
            
            char * file_path =malloc((strlen(folder_name)+strlen(dir->d_name)+2)*sizeof(char));
            
            sprintf(file_path, "%s/%s", folder_name, dir->d_name);
            lstat(file_path, &sb);

            if(op_l){
                double file_size;
                int unit = 0;
                // Mode
                print_mode(sb.st_mode, mode);
                //printf("%s ", mode);

                // Link Count
                printf("%ld ", (long) sb.st_nlink);
                printspace(dim[0]-intlen(sb.st_nlink));

                // Ownership
                passwd = getpwuid(sb.st_uid);
                printf ("\e[1;33m%s\e[0m ", passwd->pw_name);
                printspace(dim[1]-mbStrlen(passwd->pw_name));

                group = getgrgid(sb.st_gid);
                printf ("\e[0;33m%s\e[0m ", group->gr_name);
                printspace(dim[2]-mbStrlen(group->gr_name));

                //File size
                file_size = (double) sb.st_size;
                
                printf("\e[0;35m");
                if(op_h){
                    while(file_size>=1000.){
                        file_size /= 1024;
                        unit++;
                    }

                    if (file_size<10 && unit>0) {
                        printf("%.1f %cB ", file_size, units[unit]);
                    }
                    else if(file_size<10){
                        printf("%.1f B  ", file_size);
                    }
                    else if(unit>0) {
                        printspace(3-intlen(file_size));
                        printf("%.0f %cB ", file_size, units[unit]);
                    }
                    else {
                        printspace(3-intlen(file_size));
                        printf("%.0f B  ", file_size);
                    }
                }
                else {
                    printspace(dim[3]-intlen(file_size));
                    printf("%.0f ", file_size);
                }
                printf ("\e[0m ");

                // Last modification
                printf("%s ", strtok(ctime(&sb.st_mtime), "\n") );

                // Name
                print_file(dir->d_name, sb);

                printf("\n");
            }
            
        }
    }
    else {
        int line_width = 0;
        while (nb_cur--) {
            dir = dir_list[nb_file-nb_cur-1];
            line_width += mbStrlen(dir->d_name)+4;
        }
        line_width--;

        //printf("%d/%d", line_width, term_width);

        nb_cur = nb_file;
        if (line_width<=term_width){
            while (nb_cur--) {
                dir = dir_list[nb_file-nb_cur-1];
                char * file_path =malloc((strlen(folder_name)+strlen(dir->d_name)+2)*sizeof(char));
                sprintf(file_path, "%s/%s", folder_name, dir->d_name);
                lstat(file_path, &sb);
                
                print_file(dir->d_name, sb);
                printf("  ");
            }
        }
        else {
            int nb_c = ceil((double) nb_file/2.)+1;
            int al_ok = 0;
            int nb_line;
            //printf("%d\n", nb_c);
            while(!al_ok){
                nb_c--;
                al_ok=1;
                nb_line = ceil((double)nb_file/(double)nb_c);
                //printf("c=%d, l=%d\n", nb_c, nb_line);
                while ( nb_c-(nb_line*nb_c-nb_file)<nb_c-1)
                {
                    nb_c--;
                    nb_line = ceil((double)nb_file/(double)nb_c);
                    //printf("c=%d, l=%d\n", nb_c, nb_line);
                }
                
                /*
                for(int i=0; i<nb_line; i++){
                    al_ok = al_ok && (width_lines(dir_list, nb_c, nb_line, i, nb_file)<=term_width);
                }*/
                al_ok = (width_lines(dir_list, nb_c, nb_line, nb_file)<=term_width);
            }
            //printf("c=%d, l=%d\n", nb_c, nb_line);
            int dim_c[nb_c];
            for(int i=0; i<nb_c; i++){
                dim_c[i] = width_column(dir_list, nb_c, nb_line, i, nb_file);
                //printf("%d ", width_column(dir_list, nb_c, nb_line, i, nb_file));
            }
            //printf("\n");
            for(int i=0; i<nb_line; i++){
                print_line(dir_list, dim_c, nb_c, nb_line, i, nb_file);
            }

        }
    }

    if(!op_l) printf("\n");
}

void print_file(char* dir_name, struct stat sb){
    switch (sb.st_mode & S_IFMT) {
        
        // Block
        case S_IFBLK:
            printf("\e[33;40m%s\e[0m", dir_name);
            break;
        // Char
        case S_IFCHR:
            printf("\e[33;40m%s\e[0m", dir_name);
            break;
        // Directory
        case S_IFDIR:
            if(regex_test(dir_name, "^(Downloads|Téléchargements)$") == 0)
                printf("\e[34;1m󰉍 %s\e[0m", dir_name);
            else if(regex_test(dir_name, "^Documents$") == 0)
                printf("\e[34;1m󰝰 %s\e[0m", dir_name);
            else if(regex_test(dir_name, "^Pictures|Images$") == 0)
                printf("\e[34;1m󰉏 %s\e[0m", dir_name);
            else if(regex_test(dir_name, "^(Templates|Modèles)$") == 0)
                printf("\e[34;1m󱋣 %s\e[0m", dir_name);
            else if(regex_test(dir_name, "^(Videos|Vidéos)$") == 0)
                printf("\e[34;1m󱧺 %s\e[0m", dir_name);
            else if(regex_test(dir_name, "^(Music|Musique)$") == 0)
                printf("\e[34;1m󱍙 %s\e[0m", dir_name);
            else if(regex_test(dir_name, "^(Desktop|Bureau)$") == 0)
                printf("\e[34;1m󱂵 %s\e[0m", dir_name);
            else
                printf("\e[34;1m󰉋 %s\e[0m", dir_name);
            break;
        // FIFO
        case S_IFIFO:
            printf("\e[33;40m%s\e[0m", dir_name);
            break;
        // Symlink
        case S_IFLNK:
            printf("\e[36;1m󰉒 %s\e[0m", dir_name);
            if(op_l){
                printf(" 󰜴 ");

                char buf[PATH_MAX];
                char * file_path =malloc((strlen(folder_name)+strlen(dir_name)+2)*sizeof(char));
                sprintf(file_path, "%s/%s", folder_name, dir_name);
                char *res = realpath(file_path, buf);
                
                if (res) { // or: if (res != NULL)
                    stat(buf, &sb);
                    print_file(buf, sb);
                }
                //stat(file_path, &sb);
            }
            break;
        // Socket
        case S_IFSOCK:
            printf("\e[35;1m%s\e[0m", dir_name);
            break;
        // Regular file
        case S_IFREG:
            if (sb.st_mode & S_IXUSR) printf("\e[32;1m󰆍 %s\e[33;0m", dir_name);
/*
LS_COLORS='rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;
01:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;
31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:
*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:
*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:
*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:
*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:
*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:
*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:
*.mpeg=01;35:*.mpeg=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:
*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:
*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:
*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=00;36:*.au=00;36:*.flac=00;36:*.mid=00;36:*.midi=00;
36:*.mka=00;36:*.mp3=00;36:*.mpc=00;36:*.ogg=00;36:*.ra=00;36:*.wav=00;36:*.axa=00;36:*.oga=00;36:
*.spx=00;36:*.xspf=00;36:';
*/
            else{
                // Media files
                if(regex_test(dir_name, "\\.(jpg|jpeg|gif|bmp|pbm|pgm|ppm|tga|xbm|xpm|tif|tiff|png|svg|svgz|mng|pcx)$") == 0)
                    printf("\e[35;1m󰋩 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(mov|mpg|mpeg|mkv|webm|ogm|mp4|m4v|mp4v)$") == 0)
                    printf("\e[35;1m󰎁 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(vob|qt|nuv|wmv|asf|rm|rmvb|flc|avi|fli|flv|gl|dl|xcf|xwd|yuv|cgm|emf|axv|anx|ogv|ogx)$") == 0)
                    printf("\e[35;1m󰈔 %s\e[0m", dir_name);
                
                // Compressed files
                else if(regex_test(dir_name, "\\.(tar|tgz|arc|arj|taz|lha|lz4|lzh|lzma|tlz|txz|tzo|t7z|zip|z|Z|dz|gz|lrz|lz|lzo|xz|bz2|bz|tbz|tbz2|tz|deb|rpm|jar|war|ear|sar|rar|alz|ace|zoo|cpio|7z|rz|cab)$") == 0)
                    printf("\e[01;31m󰈢 %s\e[0m", dir_name);

                // Audio files
                else if(regex_test(dir_name, "\\.(aac|au|flac|mid|midi|mp3|mpc|ogg|ra|wav|axa|oga|spx|xspf)$") == 0)
                    printf("\e[00;36m %s\e[0m", dir_name);
                // PDF
                else if(regex_test(dir_name, "\\.(pdf)$") == 0)
                    printf("\e[0m󰈦 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(py|ipynb|pyc)$") == 0)
                    printf("\e[0m󰌠 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(doc|docx)$") == 0)
                    printf("\e[0m󰈬 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(xls|xlsx|csv)$") == 0)
                    printf("\e[0m󰓫 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(txt)$") == 0)
                    printf("\e[0m󰈙 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(html)$") == 0)
                    printf("\e[0m󰌝 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(css|scss)$") == 0)
                    printf("\e[0m󰌜 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(c|h)$") == 0)
                    printf("\e[0m󰙱 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(cpp)$") == 0)
                    printf("\e[0m󰙲 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(cs)$") == 0)
                    printf("\e[0m󰌛 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(js)$") == 0)
                    printf("\e[0m󰌞 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "\\.(json)$") == 0)
                    printf("\e[0m󰘦 %s\e[0m", dir_name);

                else if(regex_test(dir_name, "^makefile$") == 0)
                    printf("\e[0m󰒓 %s\e[0m", dir_name);
                else if(regex_test(dir_name, "^(README|README.md)$") == 0)
                    printf("\e[0m󰃀 %s\e[0m", dir_name);
                else printf("\e[0m󰈔 %s\e[0m", dir_name);
            }
            break;
        default:
            printf("unknown? ");
            break;
    }
}

void print_mode(mode_t m, char* mode)//Permission
{
    if(S_ISREG(m))
        printf("-");
    else if(S_ISDIR(m))
        printf("\e[34;1md\e[0m");
    else if(S_ISCHR(m))
        printf("\e[33;40mc\e[0m");
    else if(S_ISBLK(m))
        printf("\e[33;40mb\e[0m");
    else if(S_ISFIFO(m))
        printf("\e[33;40mf\e[0m");
    else if(S_ISLNK(m))
        printf("\e[36;1ml\e[0m");
    else if(S_ISSOCK(m))
        printf("\e[35;1mn\e[0m");

    // users
    printf(m&S_IRUSR?"\e[32;1mr\e[0m":"-");
    printf(m&S_IWUSR?"\e[32;1mw\e[0m":"-");
    printf(m&S_IXUSR?"\e[32;1mx\e[0m":"-");

    // group
    printf(m&S_IRGRP?"\e[32;1mr\e[0m":"-");
    printf(m&S_IWGRP?"\e[32;1mw\e[0m":"-");
    printf(m&S_IXGRP?"\e[32;1mx\e[0m":"-");

    //Other
    printf(m&S_IROTH?"\e[32;1mr\e[0m":"-");
    printf(m&S_IWOTH?"\e[32;1mw\e[0m":"-");
    printf(m&S_IXOTH?"\e[32;1mx\e[0m":"-");

    printf(" ");
}

int regex_test(char * string, char * reg){
    regex_t regE;
    int value_reg, value_res;
    value_reg = regcomp( &regE, reg, REG_EXTENDED);

    if (value_reg != 0) {
        perror("regcomp");
        exit(EXIT_FAILURE);
    }

    value_res = regexec(&regE, string, 0, NULL, 0);

    return value_res;
}

int hidden_filter(const struct dirent *dir){
    return (regex_test((char *) dir->d_name, "^\\."));
}

int get_term_width(void){
    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    //printf("columns %d\n", ws.ws_col);
    return (int) ws.ws_col;
}

int intlen(int n){
    return floor(log10((double) abs(n))) + 1;
}

void printspace(int n){
    for(int i=0; i<n; i++){
        printf(" ");
    }
}

int width_lines(struct dirent **dir_list, int nb_c, int nb_l, int nb_file){
    struct dirent *dir;
    int line_width = 0;
    int max_col = 0;
    for(int i=0; i<nb_c; i++){
        max_col = 0;
        for(int j=0; j<nb_l; j++){
            if((i*nb_l+j)<nb_file){
                dir = dir_list[i*nb_l+j];
                if(mbStrlen(dir->d_name)>max_col) max_col = mbStrlen(dir->d_name);
            }
        }
        line_width += max_col+4;
    }
    line_width--;

    return line_width;
}
int width_column(struct dirent **dir_list, int nb_c, int nb_l, int num_c, int nb_file){
    struct dirent *dir;
    int col_width = 0;
    for(int j=0; j<nb_l; j++){
        if((j+nb_l*num_c)<nb_file){
            dir = dir_list[j+nb_l*num_c];
            if (mbStrlen(dir->d_name)>col_width){
                col_width = mbStrlen(dir->d_name);
            }
        }
    }
    return col_width+4;
}
void print_line(struct dirent **dir_list, int dim_c[], int nb_c, int nb_l, int num_l, int nb_file){
    struct dirent *dir;
    struct stat sb;
    for(int i=0; i<nb_c; i++){
        if((i*nb_l+num_l)<nb_file){
            dir = dir_list[i*nb_l+num_l];
            char * file_path =malloc((strlen(folder_name)+strlen(dir->d_name)+2)*sizeof(char));
            sprintf(file_path, "%s/%s", folder_name, dir->d_name);
            lstat(file_path, &sb);
            print_file(dir->d_name, sb);
            if(i<(nb_c-1)) printspace(dim_c[i]-mbStrlen(dir->d_name)-2);
        }
    }
    printf("\n");
}


size_t mbStrlen( const char * str ) {

    size_t result = 0;
    int nbChars;
    wchar_t extractedChar;

    // Si mblen renvoi 0, alors on est arrivé à la fin de la chaîne
    while( ( nbChars = mblen( str, MB_CUR_MAX ) ) ) {

        // Si mblen renvoi -1, alors il y a eut une erreur durant le décodage de la chaîne.
        if ( nbChars == -1 ) {
            fprintf( stderr, "On ne peut pas décoder la chaîne.\n" );
            fprintf( stderr, "Vérifiez votre localisation (fonction setlocale).\n" );
            exit( -1 );
        }

        // On extrait le caractère multi-octets dans un caractère long (wide char) de type
        // wchar_t pour facilement le prendre en charge dans la fonction printf.
        mbtowc( &extractedChar, str, nbChars );

        // On affiche le caractère et sa taille en nombre d'octets
        //printf( "%lc -> %d \n", extractedChar, nbChars );

        // On passe au caractère suivant
        str += nbChars;
        result ++;
    }

    return result;
}