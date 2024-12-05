const AWS = require('aws-sdk');

const ICONS_TABLE_NAME              =  "8x8_icons";
const DISPLAY_CONFIG_REF_TABLE_NAME =  "8x8_config";

const TABLE_CONFIG_ICONREF_KEY      = 'iconRef'

const dynamo = new AWS.DynamoDB.DocumentClient({region: 'us-west-2'});

const toHexaStr = (iconData) => {
  let result = "";
  iconData.forEach((r, i) => {
    if(i % 2 == 0) {
      r = r.reverse();
    }
    r.forEach(c => result = result.concat(c, " "))
  })
  return result.slice(0, -1);
}

const getConfigValue = async (configProp) => {
  const params = {
    TableName : DISPLAY_CONFIG_REF_TABLE_NAME,
    Key: {
      prop: configProp
    }
  }
  let data = await dynamo.get(params).promise()

  return data.Item.value
}

const updateDisplayedIcon = async (icon) => {
  const params = {
    TableName: DISPLAY_CONFIG_REF_TABLE_NAME,
    Key: {
      prop: 'iconRef'
    },
    UpdateExpression: 'set #value = :r',
    ExpressionAttributeNames: {
      '#value': 'value'
    },
    ExpressionAttributeValues: {
      ':r': icon,
    },
  };

  const result = await dynamo.update(params).promise();

  return result
}

exports.handler = async (event, context) => {
    let body;
    let statusCode = '200';
    const headers = {
      'Content-Type': 'application/json',
    };

    try {
      switch (event.httpMethod) {
        case 'POST':
          const requestBody = JSON.parse(event.body);
          if (requestBody?.iconRef == null || requestBody.iconRef.length > 100) {
            return {
              statusCode: 400,
              body: JSON.stringify({ message: 'Invalid request. ' + requestBody }),
            };
          }

          await updateDisplayedIcon(event.body)

          break;

        case 'GET':

          const icon = await getConfigValue('iconRef')

          let mode = event?.queryStringParameters?.key
          if(mode === 'esp') {
            body = toHexaStr(JSON.parse(icon).body.icons[0])
          } else {
            body = icon
          }

          break;
        default:
          throw new Error(`Unsupported method "${event.httpMethod}"`);
      }
    } catch (err) {
      statusCode = '400';
      body = err.message;
    } 

  return {
    statusCode,
    body,
    headers
  };
};
