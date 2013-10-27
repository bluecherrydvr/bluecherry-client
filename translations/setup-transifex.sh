#!/usr/bin/env bash

tx init
tx set --auto-local -r bluecherry-client.bluecherry-client -s en 'translations/bluecherryclient_<lang>.ts'  -t QT  --execute

###commands:

##push resources:
#tx push -s -t

##download transactions:
#tx pull -a
