CODEBASE="http://l040101-ws02.ua.pt/"$1"/classes/"
java -Djava.rmi.server.codebase=$CODEBASE\
     -Djava.rmi.server.useCodebaseOnly=true\
     -Djava.security.policy=java.policy\
     serverSide.main.ServerControlCollectionSite 22132 l040101-ws07.ua.pt 22137