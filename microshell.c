#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define true 1
#define false 0

const int BUFF_SIZE = 255;

int startup () {
    
    system ( "clear" );
    printf ( "\nMISIEKSHELL: Projekt Systemy operacyjne\n\n" );
    printf ( "--------------------------------------\n" );
    
    return 0;
}

int helper () {

    printf ( "\nMISIEKSHELL by Michal Goralczyk\n" );
    printf ( "Type \"exit\" to exit shell.\n" );
    printf ( "Type \"cp [destination/file_to_copy] [destination/new_file]\" to copy file.\n" );
    printf ( "Type \"touch [destination/file]\" to touch file.\n" );
    printf ( "Type \"cd [directory]\" to change working directory.\n\n" );

    return 0;
    
}

int shutdown () {
    
    printf ( "\nExiting MISIEKSHELL in 3 seconds\n" );
    
    sleep( 3 );

    return 0;    
}

int cd ( char* parametry [ ] ) {

    if ( parametry [ 1 ] == NULL ){
        struct passwd *r = getpwuid ( getuid ( ) );
        const char *home = r->pw_dir;
        chdir ( home );
    } else {

        char* point = memchr ( parametry [ 1 ], '~', strlen( parametry [ 1 ] ) );

        if ( point == NULL ) {

            if ( opendir ( parametry [ 1 ] ) == NULL ) perror ("Error");
            else chdir ( parametry [ 1 ] );

        } else {
        
            struct passwd *r = getpwuid ( getuid ( ) );

            const char *home = r->pw_dir;
            chdir ( home );

            if ( strcmp ( parametry [ 1 ], "~" ) == 0 );
            else {
                char* param = point + 2;
                if ( opendir ( param ) == NULL ) perror ("B");
                else chdir ( param ); 
            }
        }  
    }

    return 0;     
}

int execute ( char* parametry [ ] ) {

    pid_t process_id = fork();

    if ( process_id == 0 ) {
        if ( execvp ( parametry [ 0 ], parametry ) == -1 ) {
            perror ( "command not found" );
            exit ( -1 );
        }
    } else wait ( NULL );
    
    return 0;
}

int touch ( char* parametry [ ] ) {

    struct timespec *czas = NULL;

    int desk = open ( parametry [ 1 ], O_CREAT|O_RDONLY, 0666 );
    
    futimens ( desk, czas );

    close ( desk );

    return 0;
}

int cp ( char* parametry [ ] ) {
 
    if ( open ( parametry [ 1 ], O_RDONLY ) == -1 ) perror ( "No such file exist!" );
    else {

        char buffer [ BUFF_SIZE ];

        int desk1 = open ( parametry [ 1 ], O_RDONLY );
        creat ( parametry [ 2 ], 0666 );
        int desk2 = open ( parametry [ 2 ], O_WRONLY );
        int bajty;
        
        while ( ( bajty = read ( desk1, &buffer, BUFF_SIZE ) ) > 0 ) write ( desk2, &buffer, bajty );

        close ( desk1 );
        close ( desk2 );
    }
    
    return 0;
}

int savehistory ( char* polecenie ) {

    struct passwd *r = getpwuid ( getuid ( ) );
    char *home = r->pw_dir;
    strcat (home, "/.history");

    int desk_his = open ( home, O_CREAT|O_WRONLY|O_APPEND, 0666 );
    write ( desk_his, polecenie, strlen ( polecenie ) );
    write ( desk_his, "\n", strlen ( "\n" ) );
    close ( desk_his );

    return 0;
}

int openhistory ( ) {

    struct passwd *r = getpwuid ( getuid ( ) );
    char *home = r->pw_dir;
    strcat (home, "/.history");
    
    char* buffor;
    int bajty;

    int desk_his = open ( home, O_CREAT|O_RDONLY, 0666 );

    while ( ( bajty = read ( desk_his, &buffor, 1 ) ) > 0 ) {
        write ( STDOUT_FILENO, &buffor, bajty );
    }

    close ( desk_his );

    return 0;
}

int main () {
    int warunek = true;
    
    char polecenie [ BUFF_SIZE ];
    char* polecenie_dyn; 
    char* user;

    char katalog [ BUFF_SIZE ];

    char key_buff;

    startup ();

    while ( warunek ) {
        
        memset ( polecenie, 0, BUFF_SIZE );
        
        getcwd ( katalog, BUFF_SIZE );

        struct passwd *r = getpwuid ( getuid ( ) );
        user = r -> pw_name; 

        printf ( "\033[1;32m[%s]\033[0m:\033[1;35m[%s]\033[0m $:", user, katalog );

        
        int i;

        for ( i = 0; i <= BUFF_SIZE; i++ ) {
            
            key_buff = getchar ();
            
            int key_buff_int = key_buff;
            
            if ( key_buff_int == 10 ) break;
            else if ( i == BUFF_SIZE ) printf ( "Exceded buffer size" );
            else polecenie [ i ] = key_buff;      
        }

        polecenie_dyn = polecenie;

        savehistory ( polecenie_dyn );

        char* parametry [20];
        char* polecenie_c = polecenie;
        char* polecenie_p = strtok( polecenie_c, " " );

        int iter = 0;

        while ( polecenie_p != NULL ) {
            parametry [ iter ] = polecenie_p;
            polecenie_p = strtok( NULL, " " );
            iter++;
        }
        parametry [ iter ] = polecenie_p;
        polecenie_p = strtok( NULL, " " );       
  
        if ( strcmp ( parametry [ 0 ], "exit" ) == 0 ) break;
        
        else if ( strcmp ( parametry [ 0 ], "help" ) == 0 ) helper (); 
        
        else if ( strcmp ( parametry [ 0 ], "cd" ) == 0 ) cd ( parametry );

        else if ( strcmp ( parametry [ 0 ], "touch" ) == 0 ) touch ( parametry );

        else if ( strcmp ( parametry [ 0 ], "cp" ) == 0 ) cp ( parametry );
        
        else if ( strcmp ( parametry [ 0 ], "history" ) == 0 ) openhistory ( );
        
        else execute ( parametry );    
    }
    
    shutdown ();
    
    exit ( EXIT_SUCCESS );
}