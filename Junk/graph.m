%This MATLAB code can be used to graph the output of the simulations

load sim.txt;

x = sim(:,2);
y = sim(:,1);

plot(x, y, 'linewidth', 2);

title('Throughput');
xlabel('distance [m]');
ylabel('Throughput');

%in the script used to output data points the following code should be added:
%     std::ofstream myfile;
%     myfile.open("sim.txt");
%     // calculations and other code goes here

%     myfile << throughput << " " << dist << std::endl;
%     myfile.close();
