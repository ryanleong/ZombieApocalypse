#! /bin/octave -qf
% script to plot runtimes from the zombie simulation.

arg_list = argv ();
input = arg_list{1};
world_size = str2num(arg_list{2});

% optional third argument
if (length(arg_list) >= 3);
    output = arg_list{3};
else
    output = ['mpi-scaling-', (num2str (world_size)), '.eps'];
endif

% the default one cannot produce the legend correctly
graphics_toolkit gnuplot


data = load (input);

% coords will tell us which rows we want to use from the data, to
% select only the rows for the current world size.
coords = find (data (:, 2) == world_size);

% We would expect that the runtime will be inversely proportional
% to the number of cores we throw at the problem, so taking 1 divided
% by the runtime should (if we have perfect scaling) produce a straight
% line. Divergence from a straight line indicates non ideal scaling.
%
% Comment this line out to plot the plain running time.
%data (coords, 4) = 1 ./ data (coords, 4);

figure;
%plot (data (coords, 3), data (coords, 4), '.b');
b = bar ([(data (coords, 4)), (data (coords, 4) .* data (coords, 1)), ((data (coords, 4) .* data (coords, 1)) .* data (coords, 7) / 100)]);
set (b(1), 'facecolor', 'g');
set (b(2), 'facecolor', 'b');
set (b(3), 'facecolor', 'r');

%title (['World size ', (num2str (world_size))]);
xlabel ('number of nodes_{number of threads}');
set (gca, 'xticklabel', {'1_{32}', '1_{64}', '4_{32}', '4_{64}', '16_{32}', '16_{64}', '64_{32}', '64_{64}', '256_{32}', '256_{64}'});

% Select one of these ylabels.
ylabel ('running time (seconds)');
%ylabel ('1 / running time in seconds');

legend ('run time ', 'total work ', 'idle work ', 'location', 'northwest');

% set font name to something better
FN = findall(gcf,'-property','FontName');
set(FN,'FontName','/usr/share/fonts/dejavu/DejaVuSerifCondensed.ttf');

grid on;
print (output, '-color', '-deps', '-S400,300');
close (gcf);


% vim: ft=octave ts=4 sw=4 et
