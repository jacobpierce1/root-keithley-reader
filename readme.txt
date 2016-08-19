The data is plotted by "plot_keithley.cpp" in root. Many command line arguments can be specified to create a customized graph. In order to do this, there is a shell script "plot" to more easily supply arguments. (This also gives the great advantage of still being able to use built in auto-complete for entering directory names by pressing tab). Here are all the possible arguments (to be explained in detail after):

    $ ./plot <directory> <leg1,...,legN> <title> <xmin, xmax, ymin, ymax> 

The only required argument is the directory to be processed; if this is supplied the legends and title will be set to defaults from the file and directory names. To run the program, execute plot with the first argument as the directory containing all files to be graphed, e.g.

   $ ./plot hamamatsu_channels/

If the files desired are in a subdirectory, this can also be handled easily:

   $ ./plot hamamatsu_channels/31C/

If legends are to be specified, they may be delimited by commas (the correct number must be specified, otherwise there will be a seg fault):

   $ ./plot hamamatsu_channels/31C/ a,b,c,d,e

The above will give a default title for plot 4, because "d" and "default" are reserved as fillers for the default case. If spaces are to be used, enter the name with underscores and they will be parsed as spaces. Also a legend argument of just "d" for multiple plots (which normally would produce a seg fault) gives a default to every legend (useful for changing later parameters without modifying legend).

For the title, just enter a string:

   $ ./plot hamamatsu_channels/31C/ a,b,c,d,e test_title

The same parsing of "d", "default", and '_' applies to the title.

Finally, the max and min values to be plotted can be specified by the last parameter. Entering "d" for any of the values will not change it. For example:

   $./plot hamamatsu_channels/31C/ d d 30,40,d,10

This will give default legend and title and will only allow x (voltage) values in [30,40] and y (current) values in [-\infty, 10]. 
