from django.shortcuts import render
from django.http.response import JsonResponse

from rest_framework.decorators import api_view
from django.views.decorators.csrf import csrf_exempt

@csrf_exempt
@api_view(['GET',])
def get_logs(request):
    with open('backend.log', 'r') as log_file:
        logs = log_file.readlines()

    return JsonResponse({'logs': logs})
