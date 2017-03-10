    function format_log_json()
    {
		$.ajaxSetup({ cache: false });

       $.ajax({
	        
			json: 'json',
	       
	        url: '/microscope_json_log_data',
	        success: function(json_data) {

		        window.json = $.parseJSON(json_data)
					  
				log = json.microscope_log.replace(new RegExp( "#", "g" ),"<br />");
				
					  
		        $('#log_details').html(log);
		  
				
				html = "";
				
				if(json.regionscan.has_run == 1) {
					if(json.regionscan.active == 1) {
						html += "Regionscan last started on " + json.regionscan.start_time + "<br />";
						var percentage = json.regionscan.percentage_complete * 100;
						html += "<br />Currently running: " + Math.round(percentage) + "% complete" + progressbar(percentage);
						$('#region_scan_details').html(html);
					}
					else {	
						html += "Regionscan last started on " + json.regionscan.start_time + "<br />";
						html += "Regionscan last completed on " + json.regionscan.end_time + "<br />";
						$('#region_scan_details').html(html);
					}
				}
				else {	
						html = "No regionscan has yet been run";
						$('#region_scan_details').html(html);
				}
					
	          },
       });
	}
	
	function format_contents()
    {
		$.ajaxSetup({ cache: false });

       $.ajax({
	        
			json: 'json',
	       
	        url: '/microscope_json_data',
	        success: function(json_data) {

		        window.json = $.parseJSON(json_data)
					  
		        $('#microscope_name').html(json.microscope_name);
		        $('#uptime').html(json.microscope_uptime); 
				  		  
				var html = "";
			
				if(json.timelapse.has_run == 1) {
					if(json.timelapse.active == 1) {
						html += "Timelapse last started on " + json.timelapse.start_time + "<br />";
						$('#timelapse_details').html(html);
					}
					else {	
						html += "Timelapse last started on " + json.timelapse.start_time + "<br />";
						html += "Timelapse last completed on " + json.timelapse.end_time + "<br />";
						$('#timelapse_details').html(html);
					}
				}
				else {	
						html = "No timelapse has yet been run";
						$('#timelapse_details').html(html);
				}
				
				html = "";
				
				if(json.regionscan.has_run == 1) {
					if(json.regionscan.active == 1) {
						html += "Regionscan last started on " + json.regionscan.start_time + "<br />";
						var percentage = json.regionscan.percentage_complete * 100;
						html += "<br />Currently running: " + Math.round(percentage) + "% complete" + progressbar(percentage);
						$('#region_scan_details').html(html);
					}
					else {	
						html += "Regionscan last started on " + json.regionscan.start_time + "<br />";
						html += "Regionscan last completed on " + json.regionscan.end_time + "<br />";
						$('#region_scan_details').html(html);
					}
				}
				else {	
						html = "No regionscan has yet been run";
						$('#region_scan_details').html(html);
				}
					
	          },
       });
	}

    function progressbar()
    {
        var progress = arguments[0];
        
        if ( progress == null )
            return null;
     
        var html = "";
            html += "<table width='200px' cellspacing='1' cellpadding='0' border='0'>";
            html += "<tr>";
      
            for( var ix = 0; ix <100; ix=ix+10 )
            {
                if( ix <progress )
                    html += "<td class='progress_foreground' nowrap='nowrap'>&nbsp;</td>";
                else
                    html += "<td class='progress_background' nowrap='nowrap'>&nbsp;</td>"; 
            }
      
            html += "</tr>";
            html += "</table>";
            return html;
    }