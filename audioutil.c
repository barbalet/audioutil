/****************************************************************

	audioutil.c

	=============================================================

    Copyright 1996-2013 Tom Barbalet. All rights reserved.

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or
	sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    This software and Noble Ape are a continuing work of Tom Barbalet,
    begun on 13 June 1996. No apes or cats were harmed in the writing
    of this software.

****************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../nobleape/sim/noble/noble.h"

n_int draw_error(n_string error_text)
{
    printf("ERROR: %s\n", error_text);
}


FILE * test_aiff(n_string response, n_int * samples)
{
    FILE * test_file = fopen(response, "rb");
    if (test_file == 0L)
    {
        (void)SHOW_ERROR("AIFF test open failed");
        return 0L;
    }
    if (samples)
    {
        samples[0] = io_file_aiff_header_check_length(test_file);
        if (samples[0] == -1)
        {
            fclose(test_file);
            return 0L;
        }
    }
    printf("Loaded %s %ld\n",response, samples[0]);
    return test_file;
}


int main(int argc, n_string argv[])
{
    n_audio  buffer_master[AUDIO_FFT_MAX_BUFFER];
    n_audio  buffer_second[AUDIO_FFT_MAX_BUFFER];
    n_audio  buffer_silence[AUDIO_FFT_MAX_BUFFER];
    FILE    *file1 = 0L;
    FILE    *file2 = 0L;
    FILE    *filew = 0L;
    n_int    length_master;
    n_int    length_second;
    n_int    initial_offset;
    n_int    loop_count;
    n_int    loop = 0;
    n_int    prev_space = 0;
    
    n_uint   power1;
    n_uint   power2;
    
    n_audio  *writeout = buffer_silence;
    
    printf("\n *** Audio Utility %s %s ***\n", SHORT_VERSION_NAME, FULL_DATE);

    if (argc != 4)
    {
        printf("ERROR: Wrong number of arguements\n");
    }
    
    file1 = test_aiff(argv[1], &length_master);
    
    if (file1 == 0L)
    {
        printf("ERROR: First file didn't open\n");
        return 0;
    }
    
    file2 = test_aiff(argv[2], &length_second);
    
    if (file2 == 0L)
    {
        fclose(file1);
        printf("ERROR: Second file didn't open\n");
        return 0;
    }
    
    filew = fopen(argv[3],"wb");
    
    if (filew == 0L)
    {
        printf("ERROR: Write file didn't work\n");
        fclose(file1);
        fclose(file2);
        return 0;
    }
    
    
    
    if (length_second < length_master)
    {
        length_master = length_second;
    }
    length_master = length_master;
    
    initial_offset = length_master & (AUDIO_FFT_MAX_BUFFER-1);
    loop_count = length_master >> AUDIO_FFT_MAX_BITS;
    
    io_file_aiff_header(filew, (loop_count+1) << AUDIO_FFT_MAX_BITS);
    
    audio_clear_output(buffer_master, AUDIO_FFT_MAX_BUFFER);
    audio_clear_output(buffer_second, AUDIO_FFT_MAX_BUFFER);
    audio_clear_output(buffer_silence, AUDIO_FFT_MAX_BUFFER);
    
    if (initial_offset != 0)
    {
        fread(&buffer_master[AUDIO_FFT_MAX_BUFFER - 1 - initial_offset], sizeof(n_audio), initial_offset, file1);
        fread(&buffer_second[AUDIO_FFT_MAX_BUFFER - 1 - initial_offset], sizeof(n_audio), initial_offset, file2);
        
        power1 = audio_power(buffer_master, AUDIO_FFT_MAX_BUFFER);
        power2 = audio_power(buffer_second, AUDIO_FFT_MAX_BUFFER);
        /*
        printf("Audio power 1 : %lu\n", power1);
        printf("Audio power 2 : %lu\n", power2);
        */
        power1 = (power1 > 50000000);
        power2 = (power2 > 50000000);
        
        if (power1 && power2)
        {
            /*audio_combine(buffer_master, buffer_second, AUDIO_FFT_MAX_BUFFER);*/
            writeout = buffer_master;
            prev_space = 0;
        }else if (power1)
        {
            writeout = buffer_master;
            prev_space = 0;
        }else if (power2)
        {
            writeout = buffer_second;
            prev_space = 0;
        }
        else
        {
            writeout = buffer_silence;
            prev_space++;
        }
        
        if (prev_space < 2)
        {
            fwrite(writeout, AUDIO_FFT_MAX_BUFFER, sizeof(n_audio), filew);
        }
    }
    
    while (loop < loop_count)
    {
        fread(buffer_master, sizeof(n_audio), AUDIO_FFT_MAX_BUFFER, file1);
        fread(buffer_second, sizeof(n_audio), AUDIO_FFT_MAX_BUFFER, file2);
        
        power1 = audio_power(buffer_master, AUDIO_FFT_MAX_BUFFER);
        power2 = audio_power(buffer_second, AUDIO_FFT_MAX_BUFFER);
        /*
         printf("Audio power 1 : %lu\n", power1);
         printf("Audio power 2 : %lu\n", power2);
         */
        power1 = (power1 > 50000000);
        power2 = (power2 > 50000000);
        
        if (power1 && power2)
        {
            //audio_combine(buffer_master, buffer_second, AUDIO_FFT_MAX_BUFFER);
            writeout = buffer_master;
            prev_space = 0;
        }else if (power1)
        {
            writeout = buffer_master;
            prev_space = 0;
        }else if (power2)
        {
            writeout = buffer_second;
            prev_space = 0;
        }
        else
        {
            writeout = buffer_silence;
            prev_space++;
        }
        
        if (prev_space < 2)
        {
            fwrite(writeout, sizeof(n_audio), AUDIO_FFT_MAX_BUFFER, filew);
        }
        
        loop++;
    }
    
    
    fclose(file1);
    fclose(file2);
    fclose(filew);
    
    return(1);
}

