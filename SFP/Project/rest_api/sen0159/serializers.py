from rest_framework import serializers
from .models import SEN0159

class SEN0159Serializer(serializers.ModelSerializer):
        class Meta:
            model = SEN0159
            fields = '__all__'

        def create(self, validated_data):
            return SEN0159.objects.create(**validated_data)
        
        def save(self):
            return super().save()
        
        def to_representation(self, instance):
            return {
                'id': instance.id,
                'co2': instance.co2,
                'time': instance.time
            }
        
        def serialize(self):
            return super().serialize()