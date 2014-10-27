#! /bin/octave -qf

arg_list = argv ();
input = arg_list{1};

% optional second argument
if (length(arg_list) >= 2);
    output = arg_list{2};
else
    output = ['mpi-weak-scaling.eps'];
endif

% the default one cannot produce the legend correctly
graphics_toolkit gnuplot

data = load (input);

% weak scaling compares the run time for a fixed problem size per thread.
% Therefore, work out the world size per thread.
data (:, 1) = (data (:, 2) .^ 2) ./ data (:, 1);

world_size = [2048, 4096, 8192, 16384];
threads = [32, 64];
colors = ['.k'; '+g'; 'ok'; 'xg'; '.r'; '+b'; 'or'; 'xb'];
color = 1;
figure;
hold on;

for t = 1:2
	for k = 1:4
	    % coords will tell us which rows we want to use from the data, to
	    % select only the rows for the current world size.
	    coords = find ((data (:, 2) == world_size (k)) .* (data (:, 3) == threads (t)));
	    loglog (data (coords, 1), data (coords, 4), colors ((t - 1) * 4 + k, :));
	end
end

xlabel ('number of cells / number of nodes');
ylabel ('runtime in seconds');
legend ('2048 x 2048 (32 threads) ', '4096 x 4096 (32 threads) ', '8192 x 8192 (32 threads) ', '16384 x 16384 (32 threads) ',
    '2048 x 2048 (64 threads) ', '4096 x 4096 (64 threads) ', '8192 x 8192 (64 threads) ', '16384 x 16384 (64 threads) ',
    'location', 'southeast');

base = 2;
tick_exponents = 0:2:100;
set(gca, 'ytick', base .^ tick_exponents);
set(gca, 'xtick', base .^ tick_exponents);
format_string = sprintf('%d^{%%d}', base);
tick_labels = num2str(tick_exponents(:), format_string);
set(gca, 'yticklabel', tick_labels);
set(gca, 'xticklabel', tick_labels);

grid on;

% set font name to something better
FN = findall(gcf,'-property','FontName');
set(FN,'FontName','/usr/share/fonts/dejavu/DejaVuSerifCondensed.ttf');

grid on;
print (output, '-color', '-deps', '-S400,300');

hold off;
