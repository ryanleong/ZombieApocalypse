
data = load ('./single_times');

% weak scaling compares the run time for a fixed problem size per thread.
% Therefore, work out the world size per thread.
data (:, 1) = (data (:, 2)) ./ data (:, 3);

world_size = [128, 256, 512, 1024, 2048, 4096, 8192, 16384];
colors = ['.k'; '.r'; '.g'; '.b'; '.m'; '.c'; '^k'; '^r'];
color = 1;
figure;
hold on;


for k = 1:8
    % coords will tell us which rows we want to use from the data, to
    % select only the rows for the current world size.
    coords = find (data (:, 2) == world_size (k));

    plot (log2 (data (coords, 1)), log2 (data (coords, 4)), colors (k, :));
end

title ('Weak Scaling');
xlabel ('log2 (world edge length/num threads)');
ylabel ('log2 (runtime in seconds)');
legend ('128x128', '256x256', '512x512', '1024x1024', '2048x2048',
  '4096x4096', '8192x8192', '16384x16384', 'location', 'northwest');
grid on;

hold off;
