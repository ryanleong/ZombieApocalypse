% script to plot runtimes from the zombie simulation.

data = load ('./single_times');


for world_size = [128, 256, 512, 1024, 2048, 4096, 8192, 16384]
    % coords will tell us which rows we want to use from the data, to
    % select only the rows for the current world size.
    coords = find (data (:, 2) == world_size);

    % We would expect that the runtime will be inversely proportional
    % to the number of cores we throw at the problem, so taking 1 divided
    % by the runtime should (if we have perfect scaling) produce a straight
    % line. Divergence from a straight line indicates non ideal scaling.
    %
    % Comment this line out to plot the plain running time.
%    data (coords, 4) = 1 ./ data (coords, 4);

    figure;
%    plot (data (coords, 3), data (coords, 4), '.b');
    bar ([(data (coords, 4)), (data (coords, 3) .* data (coords, 4))]);
    title (['World size ', (num2str (world_size))]);
    xlabel ('number of cores');

    % Select one of these ylabels.
    ylabel ('running time (seconds)');
 %   ylabel ('1 / running time in seconds');

    grid on;
    print (['scaling-', (num2str (world_size)), '.png']);
    close (gcf);
end

% vim: ft=octave ts=4 sw=4 et
