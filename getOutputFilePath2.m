function [name, suffix]=getOutputFilePath2(file, flag, dirname)
if ~exist('flag', 'var')
    flag = '';
end;
if isempty(flag)
    flag = '';
end
m_len = length(file);
name='';
for i=m_len: -1: 1
    if(file(i)=='.')
        suffix = file(i+1:m_len);
        name = file(1:i-1);
    end
end
name=strcat(dirname, name, flag, '.', suffix);
