#!/bin/sh
SVNPATH="http://192.168.46.30/svn/codebase"

usage()
{
echo
echo "Error: Usage is: $0 <revision> <tagpath>"
echo "       Revision is numeric only. 0 = HEAD revision."
echo "       Tagpath is from /tags/<tagpath>"
echo
echo "Example: $0 196 Milestones/DemoDec2010 \"Here is my comment.\""
echo "         This creates a tag from rev 196 into /tags/Milestones/DemoDec2010"
echo
exit 1
}

if [ -z "$1" ]
then
  usage
fi
if [ -z "$2" ]
then
  usage
fi
if [ -z "$3" ]
then
  usage
fi

SVNFROM="$SVNPATH"/trunk/Neuron/code
SVNTO="$SVNPATH"/tags/$2

if [ "$1" -eq "0" ]
then
  REV="-r HEAD"
  RDESC="HEAD"
else
  REV="-r $1"
  RDESC="revision $1"
fi

echo 
echo "Preparing to make tag:"
echo "  FROM: $RDESC at $SVNFROM"
echo "    TO: $SVNTO"
echo "  WITH: Checkin comment of \"$3\""
echo
echo -n "Press <ENTER> to continue or ^C to quit -->"
read Keypress

echo
echo "Creating Tag. Please wait."
echo "Executing: svn $REV copy $SVNFROM \n           $SVNTO \n           -m \"$3\"" 
svn $REV copy $SVNFROM $SVNTO -m \"$3\"
echo
echo "All done."

exit 0

