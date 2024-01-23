from django.shortcuts import render
from django.http.response import JsonResponse

from rest_framework.decorators import api_view
from django.views.decorators.csrf import csrf_exempt

import logging
from django.utils import timezone
logger = logging.getLogger(__name__)

from .models import SEN0159
from .serializers import SEN0159Serializer


@csrf_exempt
@api_view(['GET', 'POST'])
def real_time(request):
    if request.method == 'GET':
        #return last record from database
        ser = SEN0159Serializer(SEN0159.objects.last())
        return JsonResponse(ser.data, safe=False, status=200)
    
    elif request.method == 'POST':
        #create new record
        logger.info(f'GOT {request.data}')
        ser = SEN0159Serializer(data=request.data)
        if ser.is_valid():
            ser.save()
            return JsonResponse(ser.data, status=201)
        return JsonResponse(ser.errors, status=400)
    
    return JsonResponse({'message': 'Bad request'}, status=400)

@csrf_exempt
@api_view(['GET'])
def history(request, last_of=0):
    if request.method == 'GET':
        #if last_of is not specified, return all records
        if last_of == 0:
            sen0159 = SEN0159.objects.all().order_by('-id')[::-1]
        else:
            sen0159 = SEN0159.objects.all().order_by('-id')[:last_of][::-1]

        ser = SEN0159Serializer(sen0159, many=True)
        return JsonResponse(ser.data, safe=False, status=200)
    
    return JsonResponse({'message': 'Bad request'}, status=400)
