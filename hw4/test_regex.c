#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

int main(){

regex_t regex;
int reti;
// char msgbuf[100];

/* Compile regular expression */
reti = regcomp(&regex, "LOGIN: .+\\n", 0);

reti = regexec(&regex, "LOGIN: hi\n", 0, NULL, 0);

if (!reti) {
    puts("Match");
}
else if (reti == REG_NOMATCH) {
    puts("No match");
}

regfree(&regex);


}