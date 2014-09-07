function [extension, index] = getExtension(filename)
m_len = length(filename);
for i=m_len: -1: 1
    if(filename(i)=='.')
        extension = filename(i+1:m_len);
        index = i;
        break;
    end
end