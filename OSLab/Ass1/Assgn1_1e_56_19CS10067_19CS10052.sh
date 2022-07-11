VERBOSE=$2
function logerrr()
{
    [ "$VERBOSE" == "-v"] && printf "\n$@\n"
}
export REQ_HEADERS=Accept,Host;logerr "REQ_HEADERS environment variable set";curl example.com -o example.html;logerr "Fetched webpage and created example.html";curl {ip,headers}.jsontest.com;logerr "Received IP address and the headers \n";arr=(${REQ_HEADERS//,/ });for i in ${arr[*]};do echo `curl -s headers.jsontest.com|jq ".$i"`;done;logerr "Parsed the JSON response and displayed the headers in REQ_HEADERS separated by comma \n";for filename in $(ls $1);do if jq empty $1$filename;then echo $filename>>valid.txt;else echo $filename>>invalid.txt;fi;done;logerr "Checked for valid and invalid json scripts \n"

