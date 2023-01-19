TEMPLATE = subdirs
SUBDIRS = \
        Client-UI \
        Guard-Console \
        TerminalViewer \
        Server \
        ServerCtrl

Client-UI.subdir = Client/Client-UI
Guard-Console.subdir = Client/Client-Console/Guard/Guard-Console
TerminalViewer.subdir = Client/Client-Console/Viewer/TerminalViewer
Server.subdir = Server
ServerCtrl.subdir = ServerCtrl

CONFIG += ordered
