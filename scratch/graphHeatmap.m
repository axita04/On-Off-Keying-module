%MATLAB code to graph in 3d

%fid=fopen('sim2.txt');
%XYZ=fscanf(fid,'%f %f %f',[3 Inf]);
%fclose(fid);

z = dlmread('BER2.dat');

%plot3(XYZ(1,:), XYZ(2,:), XYZ(3,:));
colormap('hot');
imagesc(z);
colorbar;
%x = reshape(XYZ(1,:),201,136)';
%y = reshape(XYZ(2,:),201,136)';
%z = reshape(XYZ(3,:),201,136)';

%surf(x, y, z);

title('Throughput values with interference on whole System');
xlabel('Interferers Power [0-200 dbm]');
ylabel('Power [0-200 dbm]');
zlabel('Throughput');

%disp('Minimum: ');
%disp(min(XYZ(3,:)));

%disp('Maximum: ');


%disp(max(XYZ(3,:)));

%in the script used to output data points the following code should be added:
%     std::ofstream otherfile;
%     otherfile.open("sim2.txt");
%     // calculations and other code goes here

%     otherfile << distx << " " << disty << x.SNR()+y.SNR()+z.SNR() << std::endl;
%     otherfile.close();
